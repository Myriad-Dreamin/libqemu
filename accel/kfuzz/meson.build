kfuzz_ss = ss.source_set()
kfuzz_ss.add(files(
  'kfuzz.c',
))

specific_ss.add_all(when: ['CONFIG_SOFTMMU', 'CONFIG_POSIX'], if_true: kfuzz_ss)
