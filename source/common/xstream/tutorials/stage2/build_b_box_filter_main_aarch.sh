#build sync_b_box_filter_main
bazel build -s //xstream/tutorials/stage2:sync_b_box_filter_main  --crosstool_top="@hr_bazel_tools//rules_toolchain/toolchain:toolchain" --cpu=x2j2-aarch64 --define cpu=x2j2-aarch64 --verbose_failures   --spawn_strategy=local
