cc_library(
	name="rabit",
	#optimize = ['msse2'],
	srcs = [
		'src/allreduce_base.cc',
		'src/allreduce_robust.cc',
		'src/engine.cc',
		'src/c_api.cc',
	],
	incs = [
		'./include',
	],
	defs = [
		'USE_SSE=1',
	],
	warning='no',
)

