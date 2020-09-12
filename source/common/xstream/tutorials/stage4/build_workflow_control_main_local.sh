#build disable_method_main
bazel build -s //xstream/tutorials/stage4:disable_method_example  --verbose_failures --spawn_strategy=local

#build pass_through_method_main
bazel build -s //xstream/tutorials/stage4:pass_through_method_example  --verbose_failures --spawn_strategy=local

#build use_predefined_method_main
bazel build -s //xstream/tutorials/stage4:use_predefined_method_example  --verbose_failures --spawn_strategy=local

#build best_effort_pass_through_method_main
bazel build -s //xstream/tutorials/stage4:best_effort_pass_through_method_example  --verbose_failures --spawn_strategy=local

#build best_effort_pass_through_method_main2
bazel build -s //xstream/tutorials/stage4:best_effort_pass_through_method2_example --verbose_failures --spawn_strategy=local
