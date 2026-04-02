
export CUDA_VISIBLE_DEVICES=2
export HF_DATASETS_OFFLINE=1
export HF_HUB_OFFLINE=1
export TRANSFORMERS_OFFLINE=1
nohup lm_eval --model hf \
    --model_args pretrained=/capsule/home/huangdaiwei/test_zgb/model/dp_llm_7b_base_exp16_swish16,trust_remote_code=True \
    --tasks arc_easy,hellaswag,commonsense_qa,copa \
    --device cuda > /capsule/home/huangdaiwei/test_zgb/test_33_lm_eval_dp_llm_7b_base_exp16_swish16_B.log 2>&1 &

