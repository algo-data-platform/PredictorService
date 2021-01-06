namespace cpp2 testutil.testservice

exception TestServiceException {
  1: string message
}

service IntermHeaderService {
  i32 callAdd(1: i32 x); // forwards to TestService::add()
}

service TestService {
  i32 sumTwoNumbers(1: i32 x, 2: i32 y),

  i32 add(1: i32 x),

  oneway void addAfterDelay(1: i32 delayMs, 2: i32 x),

  oneway void onewayThrowsUnexpectedException(1: i32 delayMs),

  void throwExpectedException(1: i32 x) throws (1: TestServiceException e),

  void throwUnexpectedException(1: i32 x) throws (1: TestServiceException e),

  void sleep(1: i32 timeMs),

  void headers();

  string hello(1: string name);
}
