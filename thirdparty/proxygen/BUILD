gen_rule(
	name='trace_gen',
	srcs=[
		'proxygen/lib/utils/samples/TraceEventType.txt',
	],
	cmd='python thirdparty/proxygen/proxygen/lib/utils/gen_trace_event_constants.py \
			--output_type=cpp \
			--input_files=thirdparty/proxygen/proxygen/lib/utils/samples/TraceEventType.txt,thirdparty/proxygen/proxygen/lib/utils/samples/TraceFieldType.txt \
			--output_scope=proxygen \
			--header_path=proxygen/lib/utils \
			--install_dir=thirdparty/proxygen/proxygen/lib/utils/ \
			--fbcode_dir=thirdparty/proxygen/proxygen/lib/utils/ \
			&& cat $FIRST_SRC > $FIRST_OUT',
	outs=[
		'dummy.txt'
	]
)
gen_rule(
	name='common_header_h_gen',
	srcs=[
		'proxygen/lib/http/HTTPCommonHeaders.template.h',
	],
	cmd='./thirdparty/proxygen/proxygen/lib/http/gen_HTTPCommonHeaders.h.sh \
			thirdparty/proxygen/proxygen/lib/http/HTTPCommonHeaders.txt \
			./thirdparty/proxygen/ thirdparty/proxygen/proxygen/lib/http \
			&& cat $FIRST_SRC > $FIRST_OUT',
	outs=[
		'HTTPCommonHeaders.template.h.txt'
	]
)
gen_rule(
	name='common_header_cpp_gen',
	srcs=[
		'proxygen/lib/http/HTTPCommonHeaders.template.h',
	],
	cmd='./thirdparty/proxygen/proxygen/lib/http/gen_HTTPCommonHeaders.cpp.sh \
			thirdparty/proxygen/proxygen/lib/http/HTTPCommonHeaders.txt \
			./thirdparty/proxygen/ thirdparty/proxygen/proxygen/lib/http \
			&& cat $FIRST_SRC > $FIRST_OUT',
	outs=[
		'HTTPCommonHeaders.template.cpp.txt'
	]
)

cc_library(
	name='proxygenutils',
	srcs=[
		'proxygen/external/http_parser/http_parser_cpp.cpp',
		'proxygen/lib/utils/AsyncTimeoutSet.cpp',
		'proxygen/lib/utils/Base64.cpp',
		'proxygen/lib/utils/Exception.cpp',
		'proxygen/lib/utils/HTTPTime.cpp',
		'proxygen/lib/utils/TraceEventContext.cpp',
		'proxygen/lib/utils/ParseURL.cpp',
		'proxygen/lib/utils/TraceEvent.cpp',
		'proxygen/lib/utils/TraceEventType.cpp',
		'proxygen/lib/utils/TraceFieldType.cpp',
		'proxygen/lib/utils/RendezvousHash.cpp',
		'proxygen/lib/utils/Logging.cpp',
		'proxygen/lib/utils/CryptUtil.cpp',
		'proxygen/lib/utils/ZlibStreamCompressor.cpp',
		'proxygen/lib/utils/ZlibStreamDecompressor.cpp',
		'proxygen/lib/utils/WheelTimerInstance.cpp',
	],
	deps=[
		':trace_gen'
	],
	warning='no'
)

cc_library(
	name='proxygenservices',
	srcs=[
		'proxygen/lib/services/RequestWorker.cpp',
		'proxygen/lib/services/Service.cpp',
		'proxygen/lib/services/WorkerThread.cpp',
	],
	warning='no'
)

cc_library(
	name='proxygenhttp',
	srcs=[
		'proxygen/lib/http/HTTPCommonHeaders.cpp',
		'proxygen/lib/http/codec/CodecProtocol.cpp',
		'proxygen/lib/http/codec/compress/GzipHeaderCodec.cpp',
		'proxygen/lib/http/codec/compress/HeaderIndexingStrategy.cpp',
		'proxygen/lib/http/codec/compress/HeaderTable.cpp',
		'proxygen/lib/http/codec/compress/HPACKCodec.cpp',
		'proxygen/lib/http/codec/compress/HPACKContext.cpp',
		'proxygen/lib/http/codec/compress/HPACKDecodeBuffer.cpp',
		'proxygen/lib/http/codec/compress/HPACKDecoder.cpp',
		'proxygen/lib/http/codec/compress/HPACKDecoderBase.cpp',
		'proxygen/lib/http/codec/compress/HPACKEncodeBuffer.cpp',
		'proxygen/lib/http/codec/compress/HPACKEncoder.cpp',
		'proxygen/lib/http/codec/compress/HPACKEncoderBase.cpp',
		'proxygen/lib/http/codec/compress/HPACKHeader.cpp',
		'proxygen/lib/http/codec/compress/Huffman.cpp',
		'proxygen/lib/http/codec/compress/NoPathIndexingStrategy.cpp',
		'proxygen/lib/http/codec/compress/Logging.cpp',
		'proxygen/lib/http/codec/compress/QPACKCodec.cpp',
		'proxygen/lib/http/codec/compress/QPACKContext.cpp',
		'proxygen/lib/http/codec/compress/QPACKDecoder.cpp',
		'proxygen/lib/http/codec/compress/QPACKEncoder.cpp',
		'proxygen/lib/http/codec/compress/QPACKHeaderTable.cpp',
		'proxygen/lib/http/codec/compress/StaticHeaderTable.cpp',
		'proxygen/lib/http/codec/ErrorCode.cpp',
		'proxygen/lib/http/codec/HTTP2Codec.cpp',
		'proxygen/lib/http/codec/HTTP2Constants.cpp',
		'proxygen/lib/http/codec/HTTP2Framer.cpp',
		'proxygen/lib/http/codec/FlowControlFilter.cpp',
		'proxygen/lib/http/codec/HTTP1xCodec.cpp',
		'proxygen/lib/http/codec/HTTPChecks.cpp',
		'proxygen/lib/http/codec/HTTPCodecFactory.cpp',
		'proxygen/lib/http/codec/HTTPCodecFilter.cpp',
		'proxygen/lib/http/codec/HTTPCodecPrinter.cpp',
		'proxygen/lib/http/codec/HTTPSettings.cpp',
		'proxygen/lib/http/codec/SPDYCodec.cpp',
		'proxygen/lib/http/codec/HTTPParallelCodec.cpp',
		'proxygen/lib/http/codec/SPDYConstants.cpp',
		'proxygen/lib/http/codec/SPDYUtil.cpp',
		'proxygen/lib/http/codec/TransportDirection.cpp',
		'proxygen/lib/http/HTTPConnector.cpp',
		'proxygen/lib/http/HTTPConstants.cpp',
		'proxygen/lib/http/HTTPException.cpp',
		'proxygen/lib/http/HTTPHeaders.cpp',
		'proxygen/lib/http/HTTPMessage.cpp',
		'proxygen/lib/http/HTTPMethod.cpp',
		'proxygen/lib/http/ProxygenErrorEnum.cpp',
		'proxygen/lib/http/experimental/RFC1867.cpp',
		'proxygen/lib/http/RFC2616.cpp',
		'proxygen/lib/http/session/ByteEvents.cpp',
		'proxygen/lib/http/session/CodecErrorResponseHandler.cpp',
		'proxygen/lib/http/session/HTTPDefaultSessionCodecFactory.cpp',
		'proxygen/lib/http/session/HTTPDirectResponseHandler.cpp',
		'proxygen/lib/http/session/HTTPDownstreamSession.cpp',
		'proxygen/lib/http/session/HTTPErrorPage.cpp',
		'proxygen/lib/http/session/HTTPEvent.cpp',
		'proxygen/lib/http/session/HTTPSessionAcceptor.cpp',
		'proxygen/lib/http/session/HTTPSessionBase.cpp',
		'proxygen/lib/http/session/HTTPSession.cpp',
		'proxygen/lib/http/session/HTTPTransaction.cpp',
		'proxygen/lib/http/session/HTTPTransactionEgressSM.cpp',
		'proxygen/lib/http/session/HTTPTransactionIngressSM.cpp',
		'proxygen/lib/http/session/HTTPUpstreamSession.cpp',
		'proxygen/lib/http/session/HTTP2PriorityQueue.cpp',
		'proxygen/lib/http/session/ByteEventTracker.cpp',
		'proxygen/lib/http/session/SimpleController.cpp',
		'proxygen/lib/http/session/TransportFilter.cpp',
		'proxygen/lib/http/Window.cpp',
	],
	warning='no',
	deps=[
		':common_header_h_gen',
		':common_header_cpp_gen',
		':proxygenservices',
		':proxygenutils',
		'//thirdparty/wangle:wangle',
	]
)

# Unused
# cc_library(
# 	name='httpparser',
# 	srcs=[
# 		'proxygen/external/http_parser/http_parser_cpp.cpp',
# 	],
# 	warning='no'
# )

cc_library(
	name='proxygenhttpserver',
	srcs=[
		'proxygen/httpserver/HTTPServerAcceptor.cpp',
		'proxygen/httpserver/HTTPServer.cpp',
		'proxygen/httpserver/RequestHandlerAdaptor.cpp',
		'proxygen/httpserver/SignalHandler.cpp',
	],
	warning='no',
	deps=[
		':proxygenhttp',
		'//thirdparty/folly:folly',
		'//thirdparty/wangle:wangle'		
	]
)
