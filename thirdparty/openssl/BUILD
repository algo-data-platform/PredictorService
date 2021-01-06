cc_library(
    name = 'crypto',
    prebuilt = True, 
	link_all_symbols=True,
	deps=[
		'#pthread'
	],
)
cc_library(
    name = 'ssl',
    prebuilt = True, 
	deps=[
		':crypto'
	],
	link_all_symbols=True,
)
