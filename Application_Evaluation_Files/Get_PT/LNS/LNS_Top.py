"""
LNS_Top.py - Python reconstruction of LNS_Top.v compute flow (no pipeline regs)

Stages:
1) Segment select via segSel.segSel
2) Converter: vsrc/converter.converter + B-path: Tran_to_822 (fixed) / FLOAT_TRAN_TO_Q822 (float)
3) CSA tree via vsrc/CSA_tree.csa_tree
4) CPA_ALOGC (simple adder tree on CSA CPA channels to 32-bit) + SAT via vsrc/SAT
   + Q822_to_fix_float conversions (Tran_log_to_mn / Tran_Q822_to_float)
   + Anti-converter via vsrc/anticonverter (soft)
5) Final CPA tree with MAD via vsrc/CPA_tree_with_MAD.cpa_tree_with_mad

All inter-stage values use binary strings. Width adaptations are done explicitly.
"""

from typing import List, Dict, Any, Tuple

# Stage modules
from segSel import segSel, segSel7
from converter import converter
from anticonverter import anticonverter
from Tran_to_822 import tran_to_822, float_tran_to_q822
from CSA_tree import csa_tree
from SAT import SAT_with_TRG_VEC_overflow, SAT_with_VEC_overflow
from Q822_to_fix_float import tran_log_to_mn, tran_q822_to_float
from CPA_tree_with_MAD import cpa_tree_with_mad


def _zfill(bits: str, w: int) -> str:
    return bits.zfill(w)[-w:]


def _bin32(value: int) -> str:
    return format(value & 0xFFFFFFFF, '032b')


def _clip_to32(bits: str) -> str:
    # Take lower 32 bits (LSBs)
    return bits[-32:].zfill(32)


def lns_top(
    seg: int | str,
    x_in: str,
    y_in: str,
    n: int | str,
    float_flag: int | bool | str,
    break_points_in: List[str],
    TRG: int | bool | str = 0,
    VEC: int | bool | str = 0,
    qi: int | bool | str = 0,
    Div: int | bool | str = 0,
    TRi: int | bool | str = 0,
    power: int | bool | str = 0,

    bias_sel: str = '000000000000',
    constant_bias_in: List[str] | None = None,

    logc_in_0: List[str] | None = None,
    K_in_0: str | None = None,

    logc_in_1: List[str] | None = None,
    K_in_1:str | None = None,

    logc_in_2: List[str] | None = None,
    K_in_2: str | None = None,

    logc_in_3: List[str] | None = None,
    K_in_3: str | None = None,

    logc_in_4: List[str] | None = None,
    K_in_4: str | None = None,

    logc_in_5: List[str] | None = None,
    K_in_5: str | None = None,

    logc_in_6: List[str] | None = None,
    K_in_6: str | None = None,
    conv_type: int = 4
) -> Dict[str, Any]:
    """
    High-level functional reconstruction.

    Inputs:
      - x_in: 32-bit bitstring
      - n: int or 5-bit bitstring
      - float_flag: 0/1/bool/'0'/'1'
      - break_points_in: list of 5x32-bit bitstrings (ascending)
      - TRG, VEC: mode flags
      - log_c_list_30: optional five 30-bit constants [c1..c5], default zeros

    Returns: dict of key intermediates and final outputs
    """
    x_in = _zfill(x_in, 32)
    is_fp = (float_flag in (1, True, '1'))


    if isinstance(n, int):
        n5 = format(n & 0x1F, '05b')
    else:
        n5 = _zfill(n, 5)

    if seg == 6:
        # Stage 1: segSel + segment coefficient selection
        seg_index = segSel(x_in, is_fp, break_points_in)

        # Normalize constant_bias_in (6 x 32-bit)
        if constant_bias_in is None:
            constant_bias_in = ['0' * 32] * 6
        constant_bias_in = [s[-32:].zfill(32) for s in constant_bias_in]

        # Build bias_seg per 2-bit control in bias_sel: for j=0..5 use bits [2*j+1:2*j]
        bias_sel_bits = bias_sel.zfill(12)[-12:]
        bias_seg: List[str] = []
        for j in range(6):
            ctrl_idx_hi = 11 - (2 * j)
            ctrl_idx_lo = 10 - (2 * j)
            ctrl = bias_sel_bits[ctrl_idx_lo:ctrl_idx_hi + 1]  # two bits with [lo:hi]
            # ctrl mapping: 00->0, 01->constant_bias_in[j], 10->x_in, 11->y_in
            if ctrl == '00':
                bias_seg.append('0' * 32)
            elif ctrl == '01':
                bias_seg.append(constant_bias_in[j])
            elif ctrl == '10':
                bias_seg.append(x_in)
            else:
                bias_seg.append(_zfill(y_in, 32))

        # Select bias_custom by seg_index (0..5)
        bias_custom = bias_seg[max(0, min(5, seg_index))]

        # Prepare logc_in lists (each contains 5x32-bit strings) and K_in (30-bit)
        def norm_logc(lst: List[str] | None) -> List[str]:
            if lst is None:
                return ['0' * 32] * 5
            return [s[-32:].zfill(32) for s in lst][:5] + ['0' * 32] * max(0, 5 - len(lst))

        logc_sets = [
            norm_logc(logc_in_0),
            norm_logc(logc_in_1),
            norm_logc(logc_in_2),
            norm_logc(logc_in_3),
            norm_logc(logc_in_4),
            norm_logc(logc_in_5),
        ]
        K_list = [
            (K_in_0 or ('0' * 30)).zfill(30)[-30:],
            (K_in_1 or ('0' * 30)).zfill(30)[-30:],
            (K_in_2 or ('0' * 30)).zfill(30)[-30:],
            (K_in_3 or ('0' * 30)).zfill(30)[-30:],
            (K_in_4 or ('0' * 30)).zfill(30)[-30:],
            (K_in_5 or ('0' * 30)).zfill(30)[-30:],
        ]

        logc_custom = logc_sets[max(0, min(5, seg_index))]
        K_custom = K_list[max(0, min(5, seg_index))]
    
    elif seg == 7:
        # Stage 1: segSel + segment coefficient selection
        seg_index = segSel7(x_in, is_fp, break_points_in)
      

        # Normalize constant_bias_in (6 x 32-bit)
        if constant_bias_in is None:
            constant_bias_in = ['0' * 32] * 7
        constant_bias_in = [s[-32:].zfill(32) for s in constant_bias_in]

        # Build bias_seg per 2-bit control in bias_sel: for j=0..5 use bits [2*j+1:2*j]
        bias_sel_bits = bias_sel.zfill(14)[-14:]
        bias_seg: List[str] = []
        for j in range(7):
            ctrl_idx_hi = 13 - (2 * j)
            ctrl_idx_lo = 12 - (2 * j)
            ctrl = bias_sel_bits[ctrl_idx_lo:ctrl_idx_hi + 1]  # two bits with [lo:hi]
            # ctrl mapping: 00->0, 01->constant_bias_in[j], 10->x_in, 11->y_in
            if ctrl == '00':
                bias_seg.append('0' * 32)
            elif ctrl == '01':
                bias_seg.append(constant_bias_in[j])
            elif ctrl == '10':
                bias_seg.append(x_in)
            else:
                bias_seg.append(_zfill(y_in, 32))

        # Select bias_custom by seg_index (0..5)
        bias_custom = bias_seg[max(0, min(6, seg_index))]

        # Prepare logc_in lists (each contains 5x32-bit strings) and K_in (30-bit)
        def norm_logc(lst: List[str] | None) -> List[str]:
            if lst is None:
                return ['0' * 32] * 6
            return [s[-32:].zfill(32) for s in lst][:6] + ['0' * 32] * max(0, 6 - len(lst))

        logc_sets = [
            norm_logc(logc_in_0),
            norm_logc(logc_in_1),
            norm_logc(logc_in_2),
            norm_logc(logc_in_3),
            norm_logc(logc_in_4),
            norm_logc(logc_in_5),
            norm_logc(logc_in_6),
        ]
        K_list = [
            (K_in_0 or ('0' * 30)).zfill(30)[-30:],
            (K_in_1 or ('0' * 30)).zfill(30)[-30:],
            (K_in_2 or ('0' * 30)).zfill(30)[-30:],
            (K_in_3 or ('0' * 30)).zfill(30)[-30:],
            (K_in_4 or ('0' * 30)).zfill(30)[-30:],
            (K_in_5 or ('0' * 30)).zfill(30)[-30:],
            (K_in_6 or ('0' * 30)).zfill(30)[-30:],
        ]

        logc_custom = logc_sets[max(0, min(6, seg_index))]
        K_custom = K_list[max(0, min(6, seg_index))]

        # print_list = [
        #     "11111111111111000000001001110110",
        #     "11111111111111000000001001110111",
        #     "11111111111111000000001001111000",
        #     "11111111111111001101000111110000",
        #     "11111111111111001101000111110001", 
        #     "11111111111111001101000111110010", 
        #     "11111111111111110001100100111100",
        #     "11111111111111110001100100111101", 
        #     "11111111111111110001100100111110",
        #     "00000000000000001000011110111110", 
        #     "00000000000000001000011110111111", 
        #     "00000000000000001000011111000000", 
        #     "00000000000000100001011011000000", 
        #     "00000000000000100001011011000001", 
        #     "00000000000000100001011011000010", 
        #     "00000000000000111110100111110001",
        #     "00000000000000111110100111110010",
        #     "00000000000000111110100111110011"
        #     ]
        # if x_in in print_list:
        #     print(f"x_in: {x_in}, seg_index: {seg_index}", "bias_custom:", bias_custom, "logc_custom:", logc_custom, "K_custom:", K_custom)
        
    

    # Stage 2: converter (log path) and B-path conversion
    abs_x, log2_out = converter(x_in, n5, '1' if is_fp else '0', conv_type)
    # print("x_in:", x_in)
    # print('log2_out:', log2_out)
    # B-path (Q822: 30-bit)
    if is_fp:
        temp_B30, ov_y, uv_y = float_tran_to_q822(y_in)
    else:
        temp_B30 = tran_to_822(y_in, n5)
    temp_B30 = _zfill(temp_B30, 30)
    # print("y_in:", y_in)
    # print("temp_B30:", temp_B30)




    if TRG in (1, True, '1'):
        B30 = K_custom
    else:
        B30 = temp_B30
    
    # print("B30:", B30)
    # Set default log constants for CSA_tree to zeros (can be connected later if needed)
    c1_32 = logc_custom[0]
    c2_32 = logc_custom[1]
    c3_32 = logc_custom[2]
    c4_32 = logc_custom[3]
    c5_32 = logc_custom[4]
    # Stage 3: CSA tree (functional)
    zero_sign_k0, cpa_ch0, cpa_ch1, cpa_ch2, cpa_ch3, cpa_ch4 = csa_tree(
        A32=log2_out,
        B30=B30,
        n5=n5,
        float_flag=('1' if is_fp else '0'),
        B0_32=y_in,
        log_c1_30=c1_32[2:32],
        log_c2_30=c2_32[2:32],
        log_c3_30=c3_32[2:32],
        log_c4_30=c4_32[2:32],
        log_c5_30=c5_32[2:32],
        TRG=('1' if TRG in (1, True, '1') else '0'),
        VEC=('1' if VEC in (1, True, '1') else '0'),
    )

    # Stage 4: CPA_ALOGC (collapse channels) and SAT + conversions

    # Choose SAT path (TRi/ TRG/ VEC flags and control signals placeholders)
    TRG_b_sign = y_in[31]
    Tri_sign = [B30[6*i+5] for i in range(5)]
    alogc_stage_power_sign = '1' if (power == '1' and temp_B30[7] == '0' ) else '0'
    input_logA_zero_sign = log2_out[0:2]
    input_logB_zero_sign = zero_sign_k0[0:2]
    input_logC_zero_sign0 = c1_32[0:2]
    input_logC_zero_sign1 = c2_32[0:2]
    input_logC_zero_sign2 = c3_32[0:2]
    input_logC_zero_sign3 = c4_32[0:2]
    input_logC_zero_sign4 = c5_32[0:2]
    alogc_stage_logA_sign = log2_out[2]

    Tri_overflow_one0, TRG_overflow0, VEC_overflow0, input_anti0 = SAT_with_TRG_VEC_overflow(
        cpa_ch0,
        TRi,
        '1' if TRG in (1, True, '1') else '0',
        '1' if VEC in (1, True, '1') else '0',
        TRG_b_sign,
        Tri_sign[4],
        alogc_stage_power_sign,
        input_logA_zero_sign,
        input_logB_zero_sign,
        input_logC_zero_sign0,
        alogc_stage_logA_sign,
    )

    Tri_overflow_one1, input_anti1 = SAT_with_VEC_overflow(
        cpa_ch1,
        TRi,
        input_logA_zero_sign,
        input_logC_zero_sign1,
        alogc_stage_logA_sign,
        '0',
        Tri_sign[3],
    )
    Tri_overflow_one2, input_anti2 = SAT_with_VEC_overflow(
        cpa_ch2,
        TRi,
        input_logA_zero_sign,
        input_logC_zero_sign2,
        alogc_stage_logA_sign,
        '0',
        Tri_sign[2],
    )
    Tri_overflow_one3, input_anti3 = SAT_with_VEC_overflow(
        cpa_ch3,
        TRi,
        input_logA_zero_sign,
        input_logC_zero_sign3,
        alogc_stage_logA_sign,
        '0',
        Tri_sign[1],
    )
    Tri_overflow_one4, input_anti4 = SAT_with_VEC_overflow(
        cpa_ch4,
        TRi,
        input_logA_zero_sign,
        input_logC_zero_sign4,
        alogc_stage_logA_sign,
        '0',
        Tri_sign[0],
    )


    # Q822_to_fix_float conversions
    log32_fix = tran_log_to_mn(input_anti0[-30:], n5)
    log32_float = tran_q822_to_float(input_anti0[-30:])

    anti_out0 = anticonverter(input_anti0, is_fp, Tri_overflow_one0, n5, conv_type)
    anti_out1 = anticonverter(input_anti1, is_fp, Tri_overflow_one1, n5, conv_type)
    anti_out2 = anticonverter(input_anti2, is_fp, Tri_overflow_one2, n5, conv_type)
    anti_out3 = anticonverter(input_anti3, is_fp, Tri_overflow_one3, n5, conv_type)
    anti_out4 = anticonverter(input_anti4, is_fp, Tri_overflow_one4, n5, conv_type)


    # Stage 5: CPA_tree_with_MAD for final accumulation
    tri_result = cpa_tree_with_mad(anti_out0, anti_out1, anti_out2, anti_out3, anti_out4, bias_custom, 1 if is_fp else 0)

    return {
        'seg_index': seg_index,
        'abs_x': abs_x,
        'log2_out': log2_out,
        'B30': B30,
        'bias_custom': bias_custom,
        'logc_custom': logc_custom,
        'K_custom': K_custom,
        'cpa_channels32': [cpa_ch0, cpa_ch1, cpa_ch2, cpa_ch3, cpa_ch4],
        'input_anti0': input_anti0,
        'input_anti1': input_anti1,
        'input_anti2': input_anti2,
        'input_anti3': input_anti3,
        'input_anti4': input_anti4,
        'anti_out0': anti_out0,
        'anti_out1': anti_out1,
        'anti_out2': anti_out2,
        'anti_out3': anti_out3,
        'anti_out4': anti_out4,
        'log32_fix': log32_fix,
        'log32_float': log32_float,
        'anti_out': [anti_out0, anti_out1, anti_out2, anti_out3, anti_out4],
        'tri_result': tri_result,
    }


if __name__ == '__main__':
    import os
    
    # Configuration (same as tb_swish_fp.v)
    y_in = '00000000000000000000000000000000'
    n = '11101'
    float_flag = '1'
    # break_points_in = ['C0DC191F', 'C03DB90E', 'BF11673D', '401B2689', '40DA0848']
    break_points_in = ['11000000110111000001100100011111', '11000000001111011011100100001110', '10111111000100010110011100111101', '01000000000110110010011010001001', '01000000110110100000100001001000']
    TRG = '1'
    VEC = '0'
    qi = '1'
    Div = '0'
    TRi = '1'
    power = '0'
    bias_sel = '010101010101'
    # constant_bias_in = ['BE25760D', 'BF3039E8', '3C8A2AF1', 'B824053E', 'BEBCA7E5', 'BE298ECF']
    # constant_bias_in = [bin(int(bias, 16))[2:].zfill(32) for bias in constant_bias_in]
    constant_bias_in= ['10111110001001010111011000001101', '10111111001100000011100111101000', '00111100100010100010101011110001', '10111000001001000000010100111110', '10111110101111001010011111100101', '10111110001010011000111011001111']

    logc_in_0 = ['01111110111011011101011100011111', '01111110001010010001110010000110', '01111101000110010000101000011101', '01111011101011011101010011101110', '10000000000000000000000000000000']
    K_in_0 = '000000000100000011000010000001'
    logc_in_1 = ['01111111100100000011110011101111', '01111110111001101110111010001011', '01111101111011001110110110011110', '01111100100001111110001010000001', '10000000000000000000000000000000']
    K_in_1 = '000000000100000011000010000001'
    logc_in_2 = ['00111111110011000010011000000011', '00111111101000100010000010101010', '00111111000110111100101111011001', '00111110001101110100100111001011', '10000000000000000000000000000000']
    K_in_2 = '000000000100000011000010000001'
    logc_in_3 = ['00111111110000000000111101000011', '00111111100000000100100010110100', '01111101110010000000110110000010', '01111110100111101111010011001101', '00111101111111111010000110100111']
    K_in_3 = '000101000100000011000010000001'
    logc_in_4 = ['00111111111110110000001110010001', '00111111001011100010000101010010', '01111110101101111010111010111011', '00111101111011100000001011001001', '01111100110010000010100010110110']
    K_in_4 = '000101000100000011000010000001'
    logc_in_5 = ['00000000000001001011111101101000', '01111110001010111110001001110001', '00111101000111000000101001110101', '01111011101100010000101010010100', '10000000000000000000000000000000']
    K_in_5 = '000000000100000011000010000001'

    # break_points_in = ['C0EC79E5', 'C033149B', 'BE3CD70F', '4020CD06', '40DCFB4A']
    # break_points_in = [bin(int(bp, 16))[2:].zfill(32) for bp in break_points_in]
    # print('break_points_in:', break_points_in)
    # constant_bias_in = ['BE00A197', 'BF0A7518', '3AFBA77E', 'B8B706E2', 'BECF7DCA', 'BE23BF10']
    # constant_bias_in = [bin(int(bias, 16))[2:].zfill(32) for bias in constant_bias_in]
    # print('constant_bias_in:', constant_bias_in)

    # break_points_in: ['11000000111011000111100111100101', '11000000001100110001010010011011', '10111110001111001101011100001111', '01000000001000001100110100000110', '01000000110111001111101101001010']
    # constant_bias_in: ['10111110000000001010000110010111', '10111111000010100111010100011000', '00111010111110111010011101111110', '10111000101101110000011011100010', '10111110110011110111110111001010', '10111110001000111011111100010000']



    # # Read input file and write output
    # script_dir = os.path.dirname(os.path.abspath(__file__))
    # in_path = os.path.join(script_dir, 'input_LNS_top.txt')
    # out_path = os.path.join(script_dir, 'soft_swish_output.txt')
    
    # if not os.path.exists(in_path):
    #     print(f"Error: {in_path} not found. Run generate_LNS_input.py first.")
    #     exit(1)
    
    # with open(in_path, 'r', encoding='utf-8') as fin, open(out_path, 'w', encoding='utf-8') as fout:
    #     count = 0
    #     for line in fin:
    #         line = line.strip()
    #         if not line:
    #             continue
    #         if len(line) != 32:
    #             print(f"Warning: skipping invalid line (not 32 bits): {line}")
    #             continue
            
    #         x_in = line
    #         out = lns_top(
    #             x_in, y_in, n, float_flag, break_points_in,
    #             TRG, VEC, qi, Div, TRi, power,
    #             bias_sel, constant_bias_in,
    #             logc_in_0, K_in_0, logc_in_1, K_in_1,
    #             logc_in_2, K_in_2, logc_in_3, K_in_3,
    #             logc_in_4, K_in_4, logc_in_5, K_in_5
    #         )
    #         print('out:', out)
    #         tri_result = out['tri_result']
    #         fout.write(f"{x_in}\t{tri_result}\n")
    #         count += 1
    #         if count % 100 == 0:
    #             print(f"Processed {count} inputs...")
    
    # print(f"Completed. Wrote {out_path} with {count} results.")

