#build thread_safety_reorder_main
bazel build -s //xstream/tutorials/stage3:thread_safety_reorder_main  --verbose_failures --spawn_strategy=local
