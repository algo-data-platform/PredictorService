cc_library(
	name="dmlc",
	#optimize = ['fopenmp', 'msse2'],
	srcs = [
		'src/io/line_split.cc',
		'src/io/indexed_recordio_split.cc',
		'src/io/recordio_split.cc',
		'src/io/input_split_base.cc',
		'src/io.cc',
		'src/io/filesys.cc',
		'src/io/local_filesys.cc',
		'src/data.cc',
		'src/recordio.cc',
		'src/config.cc',
	],
	incs = [
		'./include'
	],
	defs = [
		'DDMLC_USE_HDFS=0',
		'DDMLC_USE_S3=0',
		'DDMLC_USE_AZURE=0',
	],
	warning='no',
)

