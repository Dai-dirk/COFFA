import optuna
from optuna import Study
import json
from archSpec import formatSpec_hete

def visual(obj : Study, evaluCGRA : list, buildFromExistMOdel : bool):
    df = obj.trials_dataframe(attrs=('value','params','state'))
    print(df)
    # fig1 = optuna.visualization.plot_optimization_history(obj)
    # fig2 = optuna.visualization.plot_parallel_coordinate(obj)
    # fig3 = optuna.visualization.plot_param_importances(obj)
    # fig1.show()
    # fig2.show()
    # fig3.show()
    best_params = obj.best_params
    # if best_params["num_otrack_side1"] > best_params["num_track"]:
    #     best_params["num_otrack_side1"] = best_params["num_track"] 
    # if best_params["num_otrack_side2"] > best_params["num_track"]:
    #     best_params["num_otrack_side2"] = best_params["num_track"] 
    # if best_params["num_otrack_side3"] > best_params["num_track"]:
    #     best_params["num_otrack_side3"] = best_params["num_track"] 
    # if best_params["num_otrack_side4"] > best_params["num_track"]:
    #     best_params["num_otrack_side4"] = best_params["num_track"] 
    # if best_params["num_itrack_side1"] > best_params["num_track"]:
    #     best_params["num_itrack_side1"] = best_params["num_track"] 
    # if best_params["num_itrack_side2"] > best_params["num_track"]:
    #     best_params["num_itrack_side2"] = best_params["num_track"] 
    # if best_params["num_itrack_side3"] > best_params["num_track"]:
    #     best_params["num_itrack_side3"] = best_params["num_track"] 
    # if best_params["num_itrack_side4"] > best_params["num_track"]:
    #     best_params["num_itrack_side4"] = best_params["num_track"] 
    print("\nThe Optimal Parameters are as follows: ")
    print(best_params)
    json_str = json.dumps(evaluCGRA)
    n_trail = obj.best_trial.number
    with open('history.json', 'w') as f:
        f.write(json_str)
    print("\nArea is : %d", evaluCGRA[n_trail][0])
    print("II is : %d", evaluCGRA[n_trail][1])
    print("The latency is : ", evaluCGRA[n_trail][2])
    print("The usage is : ", evaluCGRA[n_trail][3])
    print("Mapping Failture rate is : %d", evaluCGRA[n_trail][4])
    para = {
        "fgra_num_row" : 6,
        "fgra_num_colum" : 6,
        "iob_mode" : best_params["iob_mode"],
        "max_delay_cg_iob" : best_params["max_delay_cg_iob"],
        "max_delay_fg_iob" : best_params["max_delay_fg_iob"],
        "fgra_gib_num_track_cg" : best_params["fgra_gib_num_track_cg"],
        "num_itrack_per_ipin_cg" : best_params["num_itrack_per_ipin_cg"],
        "num_otrack_per_opin_cg" : best_params["num_otrack_per_opin_cg"],
        "num_ipin_per_opin_cg" : best_params["num_ipin_per_opin_cg"],
        "diag_iopin_connect_cg" : best_params["diag_iopin_connect_cg"],
        "fgra_gib_num_track_fg" : best_params["fgra_gib_num_track_fg"],
        "num_itrack_per_ipin_fg" : best_params['num_itrack_per_ipin_fg'],
        "num_otrack_per_opin_fg" : best_params["num_otrack_per_opin_fg"],
        "num_ipin_per_opin_fg" : best_params["num_ipin_per_opin_fg"],
        "diag_iopin_connect_fg" : best_params["diag_iopin_connect_fg"],
        "max_delay_cg_gpe" : best_params["max_delay_cg_gpe"],
        "max_delay_fg_gpe" : best_params["max_delay_fg_gpe"],
        "num_input_lut" : best_params["num_input_lut"],
        "num_itrack_per_ipin_cg_l" : best_params["num_itrack_per_ipin_cg_l"],  # low
        "num_otrack_per_opin_cg_l" : best_params["num_otrack_per_opin_cg_l"],
        "num_ipin_per_opin_cg_l" : best_params["num_ipin_per_opin_cg_l"],
        "diag_iopin_connect_cg_l" : best_params["diag_iopin_connect_cg_l"],
        "num_itrack_per_ipin_fg_l" : best_params["num_itrack_per_ipin_fg_l"],
        "num_otrack_per_opin_fg_l" : best_params["num_otrack_per_opin_fg_l"],
        "num_ipin_per_opin_fg_l" : best_params["num_ipin_per_opin_fg_l"],
        "diag_iopin_connect_fg_l" : best_params["diag_iopin_connect_fg_l"],
        "max_delay_cg_gpe_l" : best_params["max_delay_cg_gpe_l"],
        "max_delay_fg_gpe_l" : best_params["max_delay_fg_gpe_l"],
        "num_input_lut_l" : best_params["num_input_lut_l"],
        "fgra_gpe_fg_rows" : [0,1,2,3,4,5],
        "fgra_gpe_fg_columns" : [0,1,2,3,4,5],
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
    spec = formatSpec_hete(para)
    with open('best_spec.json', 'w') as f:
        json.dump(spec, f, indent=4)

    # if buildFromExistMOdel :
    #     json_str = json.dumps(evaluCGRA)
    #     n_trail = obj.best_trial.number
    #     with open('history.json', 'w') as f:
    #         f.write(json_str)
    #     print("\nArea is : %d", evaluCGRA[n_trail][0])
    #     print("II is : %d", evaluCGRA[n_trail][1])
    #     print("The latency is : ", evaluCGRA[n_trail][2])
    #     print("The usage is : ", evaluCGRA[n_trail][3])
    #     print("Mapping Failture rate is : %d", evaluCGRA[n_trail][4])
        
    # else:
    #     json_str = json.dumps(evaluCGRA)
    #     n_trail = obj.best_trial.number
    #     with open('history.json', 'w') as f:
    #         f.write(json_str)
    #     print("\nArea is : %d", evaluCGRA[n_trail][0])
    #     print("II is : %d", evaluCGRA[n_trail][1])
    #     print("The latency is : ", evaluCGRA[n_trail][2])
    #     print("The usage is : ", evaluCGRA[n_trail][3])
    #     print("Mapping Failture rate is : %d", evaluCGRA[n_trail][4])
        
