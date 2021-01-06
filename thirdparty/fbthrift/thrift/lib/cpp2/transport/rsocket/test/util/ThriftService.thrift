namespace cpp2 testutil.testservice

struct Message {
  1: string message
  2: i64 timestamp
}

exception Error {
}

exception FirstEx {
  1: i64 errCode
  2: string errMsg
}

exception SecondEx {
  1: i64 errCode
}

service StreamService {
  // Generate numbers between `from` to `to`.
  stream i32 range(1: i32 from, 2: i32 to);

  // As long as the client consumes, the server will send messages
  stream Message listen(1: string sender);

  // These method will not be overiden, so the default implementation will be
  // used. If client calls these methods, it should not cause any crash and it
  // should end gracefully
  stream Message nonImplementedStream(1: string sender);

  stream Message returnNullptr();

  i32, stream Message throwError() throws (1: Error ex)

  i32, stream i32 leakCheck(1: i32 from, 2: i32 to);
  i32 instanceCount();

  i32, stream i32 sleepWithResponse(1: i32 timeMs);
  stream i32 sleepWithoutResponse(1: i32 timeMs);

  i32, stream i32 streamNever();

  // Simple chat scenario
  void sendMessage(1: i32 messageId, 2: bool complete, 3: bool error);
  stream i32 registerToMessages();

  stream Message streamThrows(1: i32 whichEx)
      throws (1: SecondEx e) stream throws (1: FirstEx e);

  i32, stream Message responseAndStreamThrows(1: i32 whichEx)
      throws (1: SecondEx e) stream throws (1: FirstEx e);
}

# OldVersion and NewVersion services will be used to test the behavior
# when the service functions change
service OldVersion {
  // Unchanged methods
  i32 AddOne(1: i32 number);
  stream i32 Range(1: i32 from, 2: i32 length);
  i32, stream i32 RangeAndAddOne(1: i32 from, 2: i32 length, 3: i32 number);

  // This method is deleted in the NewVersion
  void DeletedMethod();
  stream Message DeletedStreamMethod();
  Message, stream Message DeletedResponseAndStreamMethod();

  // This streaming method is going to be changed to a Request&Response method
  stream Message StreamToRequestResponse();

  Message, stream Message ResponseandStreamToRequestResponse();

  // This Request&Response method is going to be changed to a streaming method
  Message RequestResponseToStream();

  Message RequestResponseToResponseandStream();
}

service NewVersion {
  i32 AddOne(1: i32 number);
  stream i32 Range(1: i32 from, 2: i32 length);
  i32, stream i32 RangeAndAddOne(1: i32 from, 2: i32 length, 3: i32 number);
  void StreamToRequestResponse();
  void ResponseandStreamToRequestResponse();
  stream Message RequestResponseToStream();
  Message, stream Message RequestResponseToResponseandStream();
}
