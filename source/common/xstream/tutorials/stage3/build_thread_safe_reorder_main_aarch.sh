#build thread_safety_reorder_main
bazel build -s //xstream/tutorials/stage3:thread_safety_reorder_main  --crosstool_top="@hr_bazel_tools//rules_toolchain/toolchain:toolchain" --cpu=x2j2-aarch64 --define cpu=x2j2-aarch64 --verbose_failures   --spawn_strategy=local

