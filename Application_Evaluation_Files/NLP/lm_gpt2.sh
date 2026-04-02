
export CUDA_VISIBLE_DEVICES=3
# export HF_DATASETS_OFFLINE=1
# export HF_HUB_OFFLINE=1
# export TRANSFORMERS_OFFLINE=1
nohup lm_eval --model hf \
    --model_args pretrained=/home/gbzou/models/gpt2-xl-exp16-gelu16,trust_remote_code=True,device_map=False \
    --tasks arc_easy,hellaswag,commonsense_qa,copa \
    --device cuda > /home/gbzou/models/lm_eval_gpt2-xl-exp16-gelu16_C.log 2>&1 &

