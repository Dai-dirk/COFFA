import optuna

class Scaler:
    def __init__(self):
        self.min_area = 100000
        self.max_area = 1000000
        self.min_ii = 70
        self.max_ii = 100
        self.min_lat = 1
        self.max_lat = 35

    def normalize(self, area, ii, latency):

        n_area = (area - self.min_area) / (self.max_area - self.min_area)
        n_ii = (ii - self.min_ii) / (self.max_ii - self.min_ii)
        n_lat = (latency - self.min_lat) / (self.max_lat - self.min_lat)

        return [n_area, n_ii, n_lat]


def paramspace(trial: optuna.Trial) -> dict:
    fgra_num_row = trial.suggest_int('fgra_num_row', 6, 6)
    fgra_num_colum = trial.suggest_int('fgra_num_colum', 6, 6)
    
    iob_mode = trial.suggest_int('iob_mode', 3, 3)
    max_delay_cg_iob = trial.suggest_int('max_delay_cg_iob', 2, 8)
    max_delay_fg_iob =  trial.suggest_int('max_delay_fg_iob', 8, 32)
    
    fgra_gib_num_track_cg =  trial.suggest_int('fgra_gib_num_track_cg', 1, 2)
    num_itrack_per_ipin_cg = trial.suggest_int('num_itrack_per_ipin_cg', 2, 6)
    num_otrack_per_opin_cg = trial.suggest_int('num_otrack_per_opin_cg', 2, 6)
    num_ipin_per_opin_cg = trial.suggest_int('num_ipin_per_opin_cg', 2, 6)
    diag_iopin_connect_cg = trial.suggest_int('diag_iopin_connect_cg', 0, 1)
    
    fgra_gib_num_track_fg =  trial.suggest_int('fgra_gib_num_track_fg', 1, 3)
    num_itrack_per_ipin_fg = trial.suggest_int('num_itrack_per_ipin_fg', 2, 6)
    num_otrack_per_opin_fg = trial.suggest_int('num_otrack_per_opin_fg', 2, 6)
    num_ipin_per_opin_fg = trial.suggest_int('num_ipin_per_opin_fg', 2, 6)
    diag_iopin_connect_fg = trial.suggest_int('diag_iopin_connect_fg', 0, 1)

    max_delay_cg_gpe = trial.suggest_int('max_delay_cg_gpe', 2, 8)
    max_delay_fg_gpe =  trial.suggest_int('max_delay_fg_gpe', 16, 32)
    num_input_lut = trial.suggest_int('num_input_lut', 2, 4)

    # fgra_num_row = trial.suggest_int('fgra_num_row', 5, 5)
    # fgra_num_colum = trial.suggest_int('fgra_num_colum', 6, 6)
    
    # iob_mode = trial.suggest_int('iob_mode', 3, 3)
    # max_delay_cg_iob = trial.suggest_int('max_delay_cg_iob', 8, 8)
    # max_delay_fg_iob =  trial.suggest_int('max_delay_fg_iob', 32, 32)
    
    # fgra_gib_num_track_cg =  trial.suggest_int('fgra_gib_num_track_cg', 2, 2)
    # num_itrack_per_ipin_cg = trial.suggest_int('num_itrack_per_ipin_cg', 4, 4)
    # num_otrack_per_opin_cg = trial.suggest_int('num_otrack_per_opin_cg', 4, 4)
    # num_ipin_per_opin_cg = trial.suggest_int('num_ipin_per_opin_cg', 4, 4)
    # diag_iopin_connect_cg = trial.suggest_int('diag_iopin_connect_cg', 1, 1)
    
    # fgra_gib_num_track_fg =  trial.suggest_int('fgra_gib_num_track_fg', 2, 2)
    # num_itrack_per_ipin_fg = trial.suggest_int('num_itrack_per_ipin_fg', 4, 4)
    # num_otrack_per_opin_fg = trial.suggest_int('num_otrack_per_opin_fg', 4, 4)
    # num_ipin_per_opin_fg = trial.suggest_int('num_ipin_per_opin_fg', 4, 4)
    # diag_iopin_connect_fg = trial.suggest_int('diag_iopin_connect_fg', 1, 1)

    # max_delay_cg_gpe = trial.suggest_int('max_delay_cg_gpe', 8, 8)
    # max_delay_fg_gpe =  trial.suggest_int('max_delay_fg_gpe', 32, 32)
    # num_input_lut = trial.suggest_int('num_input_lut', 4, 4)

    # num_otrack_side1 = num_track if num_otrack_side1>num_track else num_otrack_side1
    # num_otrack_side2 = num_track if num_otrack_side2>num_track else num_otrack_side2
    # num_otrack_side3 = num_track if num_otrack_side3>num_track else num_otrack_side3
    # num_otrack_side4 = num_track if num_otrack_side4>num_track else num_otrack_side4
    # num_itrack_side1 = num_track if num_itrack_side1>num_track else num_itrack_side1
    # num_itrack_side2 = num_track if num_itrack_side2>num_track else num_itrack_side2
    # num_itrack_side3 = num_track if num_itrack_side3>num_track else num_itrack_side3
    # num_itrack_side4 = num_track if num_itrack_side4>num_track else num_itrack_side4

    fgra_gpe_fg_rows = [i for i in range(fgra_num_row)]
    fgra_gpe_fg_columns = [j for j in range(fgra_num_colum)]
    
    para = {
        "fgra_num_row" : fgra_num_row,
        "fgra_num_colum" : fgra_num_colum,
        "iob_mode" : iob_mode,
        "max_delay_cg_iob" : max_delay_cg_iob,
        "max_delay_fg_iob" : max_delay_fg_iob,
        "fgra_gib_num_track_cg" : fgra_gib_num_track_cg,
        "num_itrack_per_ipin_cg" : num_itrack_per_ipin_cg,
        "num_otrack_per_opin_cg" : num_otrack_per_opin_cg,
        "num_ipin_per_opin_cg" : num_ipin_per_opin_cg,
        "diag_iopin_connect_cg" : diag_iopin_connect_cg,
        "fgra_gib_num_track_fg" : fgra_gib_num_track_fg,
        "num_itrack_per_ipin_fg" : num_itrack_per_ipin_fg,
        "num_otrack_per_opin_fg" : num_otrack_per_opin_fg,
        "num_ipin_per_opin_fg" : num_ipin_per_opin_fg,
        "diag_iopin_connect_fg" : diag_iopin_connect_fg,
        "max_delay_cg_gpe" : max_delay_cg_gpe,
        "max_delay_fg_gpe" : max_delay_fg_gpe,
        "num_input_lut" : num_input_lut,
        "fgra_gpe_fg_rows" : fgra_gpe_fg_rows,
        "fgra_gpe_fg_columns" : fgra_gpe_fg_columns,
        "fgra_cfg_blk_offset" : 3,
        "fgra_gib_track_reged_mode_cg" : 1,
        "fgra_gpe_num_reg_rf_for_lut" : 1,
        "ls_stream_queue_depth" : 2,
        "spad_bank_lg_size" : 14,
        "fgra_iob_lg_max_lat" : 10,
        "fgra_cfg_sram_add_reg" : False,
        "fgra_iob_sram_addr_width" : 16,
        "fgra_exe_lg_max_ii" : 4,
        "spad_data_width" : 128,
        "system_bus_beat_bits" : 128,
        "fgra_gpe_out_to_dir" : [ 4, 5, 7, 6 ],
        "fgra_data_width" : 32,
        "fgra_iob_sram_banks_coalesce" : 4,
        "fgra_gib_diag_iopin_connect_fg" : True,
        "dma_lg_max_burst_size" : 6,
        "rs_exe_queue_depth" : 4,
        "operation_set_filename" : "operations.json",
        "dumpOperationSet" : True,
        "fgra_iob_lg_max_cycles" : 13,
        "fgra_exe_lg_max_execute_cycles" : 16,
        "rs_cmd_queue_depth" : 16,
        "fgra_iob_sram_add_reg" : True,
        "fgra_gpe_num_input_lut" : 3,
        "rs_store_queue_depth" : 8,
        "fgra_max_delay_fg" : 8,
        "fgra_gpe_in_from_dir" : [ 4, 5, 7, 6 ],
        "fgra_gib_diag_iopin_connect_cg" : True,
        "fgra_iob_lg_max_stride" : 13,
        "fgra_iob_has_io_fg" : True,
        "tlb_num_ways" : 32,
        "fgra_cfg_data_width" : 32,
        "fgra_max_delay_cg" : 4,
        "fgra_gpe_num_reg_rf_for_alu" : 1,
        "spad_cfg_lg_size" : 14,
        "spad_addr_num" : 9,
        "fgra_adg_filename" : "fgra_adg.json",
        "id_width" : 8,
        "dma_num_req_in_flight" : 8,
        "tlb_is_shared" : True,
        "fgra_iob_num_sides" : 2,
        "fgra_iob_lg_max_ii" : 4,
        "fgra_cfg_addr_width" : 13,
        "fgra_exe_lg_max_loop_cycles" : 10,
        "fgra_iob_mode" : 3,
        "dumpADG" : True,
        "rs_load_queue_depth" : 8,
        "fgra_gib_connect_flexibility_cg" : {
        "num_otrack_per_opin" : 4,
        "num_itrack_per_ipin" : 2,
        "num_ipin_per_opin" : 4
        },
        "fgra_iob_sram_has_mask" : False,
        "fgra_gib_connect_flexibility_fg" : {
            "num_otrack_per_opin" : 4,
            "num_itrack_per_ipin" : 2,
            "num_ipin_per_opin" : 4
        },
        "fgra_gib_track_reged_mode_fg" : 1,
        "fgra_cfg_addr_width_align" : 16,
        "fgra_gpe_operations" : ["PASS", "ADD", "SUB", "MUL", 
                                "UDIV", "AND", "OR", "XOR", "SHL", 
                                "ASHR", "EQ", "ULT", "SEL", "NOT", 
                                "SLT", "ACC", "CACC",  "CIACC", 
                                "CDIACC", "ISEL", "CISEL"],
        "spad_num_banks" : 16
    }
    return para

def paramspace_hete(trial: optuna.Trial) -> dict:
    fgra_num_row = trial.suggest_int('fgra_num_row', 6, 6)
    fgra_num_colum = trial.suggest_int('fgra_num_colum', 6, 6)
    
    iob_mode = trial.suggest_int('iob_mode', 3, 3)
    max_delay_cg_iob = trial.suggest_int('max_delay_cg_iob', 2, 8)
    max_delay_fg_iob =  trial.suggest_int('max_delay_fg_iob', 8, 22)
    
    fgra_gib_num_track_cg =  trial.suggest_int('fgra_gib_num_track_cg', 1, 2)
    num_itrack_per_ipin_cg = trial.suggest_int('num_itrack_per_ipin_cg', 2, 4)
    num_otrack_per_opin_cg = trial.suggest_int('num_otrack_per_opin_cg', 2, 4)
    num_ipin_per_opin_cg = trial.suggest_int('num_ipin_per_opin_cg', 2, 4)
    diag_iopin_connect_cg = trial.suggest_int('diag_iopin_connect_cg', 0, 1)
    num_itrack_per_ipin_cg_l = trial.suggest_int('num_itrack_per_ipin_cg_l', 0, 4)
    num_otrack_per_opin_cg_l = trial.suggest_int('num_otrack_per_opin_cg_l', 0, 4)
    num_ipin_per_opin_cg_l = trial.suggest_int('num_ipin_per_opin_cg_l', 0, 4)
    diag_iopin_connect_cg_l = trial.suggest_int('diag_iopin_connect_cg_l', 0, 1)
    
    fgra_gib_num_track_fg =  trial.suggest_int('fgra_gib_num_track_fg', 1, 2)
    num_itrack_per_ipin_fg = trial.suggest_int('num_itrack_per_ipin_fg', 2, 4)
    num_otrack_per_opin_fg = trial.suggest_int('num_otrack_per_opin_fg', 2, 4)
    num_ipin_per_opin_fg = trial.suggest_int('num_ipin_per_opin_fg', 2, 4)
    diag_iopin_connect_fg = trial.suggest_int('diag_iopin_connect_fg', 0, 1)
    num_itrack_per_ipin_fg_l = trial.suggest_int('num_itrack_per_ipin_fg_l', 2, 4)
    num_otrack_per_opin_fg_l = trial.suggest_int('num_otrack_per_opin_fg_l', 2, 4)
    num_ipin_per_opin_fg_l = trial.suggest_int('num_ipin_per_opin_fg_l', 2, 4)
    diag_iopin_connect_fg_l = trial.suggest_int('diag_iopin_connect_fg_l', 0, 1)

    max_delay_cg_gpe = trial.suggest_int('max_delay_cg_gpe', 4, 8)
    max_delay_fg_gpe =  trial.suggest_int('max_delay_fg_gpe', 12, 24)
    max_delay_cg_gpe_l = trial.suggest_int('max_delay_cg_gpe_l', 4, 8)
    max_delay_fg_gpe_l =  trial.suggest_int('max_delay_fg_gpe_l', 10, 24)
    num_input_lut = trial.suggest_int('num_input_lut', 3, 3)
    num_input_lut_l = trial.suggest_int('num_input_lut_l', 0, 3)

    # num_otrack_side1 = num_track if num_otrack_side1>num_track else num_otrack_side1
    # num_otrack_side2 = num_track if num_otrack_side2>num_track else num_otrack_side2
    # num_otrack_side3 = num_track if num_otrack_side3>num_track else num_otrack_side3
    # num_otrack_side4 = num_track if num_otrack_side4>num_track else num_otrack_side4
    # num_itrack_side1 = num_track if num_itrack_side1>num_track else num_itrack_side1
    # num_itrack_side2 = num_track if num_itrack_side2>num_track else num_itrack_side2
    # num_itrack_side3 = num_track if num_itrack_side3>num_track else num_itrack_side3
    # num_itrack_side4 = num_track if num_itrack_side4>num_track else num_itrack_side4

    fgra_gpe_fg_rows = [i for i in range(fgra_num_row)]
    fgra_gpe_fg_columns = [j for j in range(fgra_num_colum)]
    
    para = {
        "fgra_num_row" : fgra_num_row,
        "fgra_num_colum" : fgra_num_colum,
        "iob_mode" : iob_mode,
        "max_delay_cg_iob" : max_delay_cg_iob,
        "max_delay_fg_iob" : max_delay_fg_iob,
        "fgra_gib_num_track_cg" : fgra_gib_num_track_cg,
        "num_itrack_per_ipin_cg" : num_itrack_per_ipin_cg,
        "num_otrack_per_opin_cg" : num_otrack_per_opin_cg,
        "num_ipin_per_opin_cg" : num_ipin_per_opin_cg,
        "diag_iopin_connect_cg" : diag_iopin_connect_cg,
        "fgra_gib_num_track_fg" : fgra_gib_num_track_fg,
        "num_itrack_per_ipin_fg" : num_itrack_per_ipin_fg,
        "num_otrack_per_opin_fg" : num_otrack_per_opin_fg,
        "num_ipin_per_opin_fg" : num_ipin_per_opin_fg,
        "diag_iopin_connect_fg" : diag_iopin_connect_fg,
        "max_delay_cg_gpe" : max_delay_cg_gpe,
        "max_delay_fg_gpe" : max_delay_fg_gpe,
        "num_input_lut" : num_input_lut,
        "num_itrack_per_ipin_cg_l" : num_itrack_per_ipin_cg_l,  # low
        "num_otrack_per_opin_cg_l" : num_otrack_per_opin_cg_l,
        "num_ipin_per_opin_cg_l" : num_ipin_per_opin_cg_l,
        "diag_iopin_connect_cg_l" : diag_iopin_connect_cg_l,
        "num_itrack_per_ipin_fg_l" : num_itrack_per_ipin_fg_l,
        "num_otrack_per_opin_fg_l" : num_otrack_per_opin_fg_l,
        "num_ipin_per_opin_fg_l" : num_ipin_per_opin_fg_l,
        "diag_iopin_connect_fg_l" : diag_iopin_connect_fg_l,
        "max_delay_cg_gpe_l" : max_delay_cg_gpe_l,
        "max_delay_fg_gpe_l" : max_delay_fg_gpe_l,
        "num_input_lut_l" : num_input_lut_l,
        "fgra_gpe_fg_rows" : fgra_gpe_fg_rows,
        "fgra_gpe_fg_columns" : fgra_gpe_fg_columns,
        "fgra_cfg_blk_offset" : 3,
        "fgra_gib_track_reged_mode_cg" : 1,
        "fgra_gpe_num_reg_rf_for_lut" : 1,
        "ls_stream_queue_depth" : 2,
        "spad_bank_lg_size" : 14,
        "fgra_iob_lg_max_lat" : 10,
        "fgra_cfg_sram_add_reg" : False,
        "fgra_iob_sram_addr_width" : 16,
        "fgra_exe_lg_max_ii" : 4,
        "spad_data_width" : 128,
        "system_bus_beat_bits" : 128,
        "fgra_gpe_out_to_dir" : [ 4, 5, 7, 6 ],
        "fgra_data_width" : 32,
        "fgra_iob_sram_banks_coalesce" : 4,
        "fgra_gib_diag_iopin_connect_fg" : True,
        "dma_lg_max_burst_size" : 6,
        "rs_exe_queue_depth" : 4,
        "operation_set_filename" : "operations.json",
        "dumpOperationSet" : True,
        "fgra_iob_lg_max_cycles" : 13,
        "fgra_exe_lg_max_execute_cycles" : 16,
        "rs_cmd_queue_depth" : 16,
        "fgra_iob_sram_add_reg" : True,
        "fgra_gpe_num_input_lut" : 3,
        "rs_store_queue_depth" : 8,
        "fgra_max_delay_fg" : 8,
        "fgra_gpe_in_from_dir" : [ 4, 5, 7, 6 ],
        "fgra_gib_diag_iopin_connect_cg" : True,
        "fgra_iob_lg_max_stride" : 13,
        "fgra_iob_has_io_fg" : True,
        "tlb_num_ways" : 32,
        "fgra_cfg_data_width" : 32,
        "fgra_max_delay_cg" : 4,
        "fgra_gpe_num_reg_rf_for_alu" : 1,
        "spad_cfg_lg_size" : 14,
        "spad_addr_num" : 9,
        "fgra_adg_filename" : "fgra_adg.json",
        "id_width" : 8,
        "dma_num_req_in_flight" : 8,
        "tlb_is_shared" : True,
        "fgra_iob_num_sides" : 2,
        "fgra_iob_lg_max_ii" : 4,
        "fgra_cfg_addr_width" : 13,
        "fgra_exe_lg_max_loop_cycles" : 10,
        "fgra_iob_mode" : 3,
        "dumpADG" : True,
        "rs_load_queue_depth" : 8,
        "fgra_gib_connect_flexibility_cg" : {
        "num_otrack_per_opin" : 4,
        "num_itrack_per_ipin" : 2,
        "num_ipin_per_opin" : 4
        },
        "fgra_iob_sram_has_mask" : False,
        "fgra_gib_connect_flexibility_fg" : {
            "num_otrack_per_opin" : 4,
            "num_itrack_per_ipin" : 2,
            "num_ipin_per_opin" : 4
        },
        "fgra_gib_track_reged_mode_fg" : 1,
        "fgra_cfg_addr_width_align" : 16,
        "fgra_gpe_operations" : ["PASS", "ADD", "SUB", "MUL", 
                                "UDIV", "AND", "OR", "XOR", "SHL", 
                                "ASHR", "EQ", "ULT", "SEL", "NOT", 
                                "SLT", "ACC", "CACC",  "CIACC", 
                                "CDIACC", "ISEL", "CISEL"],
        "spad_num_banks" : 16
    }
    return para

def usrWeighted() -> tuple:
    with open('./sampleEva/area.txt', 'r') as a:
        area = float(a.read().strip())
    with open('./sampleEva/ii.txt', 'r') as i:   
        ii = float(i.read().strip())
    with open('./sampleEva/mappingFailureRate.txt', 'r') as m:   
        mappingFailureRate = float(m.read().strip())
    with open('./sampleEva/latency.txt', 'r') as t:   
        latency = float(t.read().strip())
    with open('./sampleEva/usage.txt', 'r') as t:   
        usage = float(t.read().strip())

    scalar = Scaler()
    processed = scalar.normalize(area, ii, latency)
    weight = [13, 0.3, 50, 0.1, 200]
    comprehensive = weight[0]*processed[0] + weight[1]*processed[1] + \
                    weight[2]*processed[2] + weight[3]*(1-usage) + \
                    weight[4]*mappingFailureRate
    result = [area, ii, latency, usage, mappingFailureRate]
    print(result)
    return (comprehensive,result)
