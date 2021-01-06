// Autogenerated by Thrift Compiler (facebook)
// DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
// @generated

package module

import (
	"bytes"
	"sync"
	"fmt"
	thrift "github.com/facebook/fbthrift-go"
)

// (needed to ensure safety because of naive import list construction.)
var _ = thrift.ZERO
var _ = fmt.Printf
var _ = sync.Mutex{}
var _ = bytes.Equal

type MyRoot interface {
  DoRoot() (err error)
}

type MyRootClient struct {
  Transport thrift.Transport
  ProtocolFactory thrift.ProtocolFactory
  InputProtocol thrift.Protocol
  OutputProtocol thrift.Protocol
  SeqId int32
}

func (client *MyRootClient) Close() error {
  return client.Transport.Close()
}

func NewMyRootClientFactory(t thrift.Transport, f thrift.ProtocolFactory) *MyRootClient {
  return &MyRootClient{Transport: t,
    ProtocolFactory: f,
    InputProtocol: f.GetProtocol(t),
    OutputProtocol: f.GetProtocol(t),
    SeqId: 0,
  }
}

func NewMyRootClient(t thrift.Transport, iprot thrift.Protocol, oprot thrift.Protocol) *MyRootClient {
  return &MyRootClient{Transport: t,
    ProtocolFactory: nil,
    InputProtocol: iprot,
    OutputProtocol: oprot,
    SeqId: 0,
  }
}

func (p *MyRootClient) DoRoot() (err error) {
  if err = p.sendDoRoot(); err != nil { return }
  return p.recvDoRoot()
}

func (p *MyRootClient) sendDoRoot()(err error) {
  oprot := p.OutputProtocol
  if oprot == nil {
    oprot = p.ProtocolFactory.GetProtocol(p.Transport)
    p.OutputProtocol = oprot
  }
  p.SeqId++
  if err = oprot.WriteMessageBegin("do_root", thrift.CALL, p.SeqId); err != nil {
      return
  }
  args := MyRootDoRootArgs{
  }
  if err = args.Write(oprot); err != nil {
      return
  }
  if err = oprot.WriteMessageEnd(); err != nil {
      return
  }
  return oprot.Flush()
}


func (p *MyRootClient) recvDoRoot() (err error) {
  iprot := p.InputProtocol
  if iprot == nil {
    iprot = p.ProtocolFactory.GetProtocol(p.Transport)
    p.InputProtocol = iprot
  }
  method, mTypeId, seqId, err := iprot.ReadMessageBegin()
  if err != nil {
    return
  }
  if method != "do_root" {
    err = thrift.NewApplicationException(thrift.WRONG_METHOD_NAME, "do_root failed: wrong method name")
    return
  }
  if p.SeqId != seqId {
    err = thrift.NewApplicationException(thrift.BAD_SEQUENCE_ID, "do_root failed: out of sequence response")
    return
  }
  if mTypeId == thrift.EXCEPTION {
    error0 := thrift.NewApplicationException(thrift.UNKNOWN_APPLICATION_EXCEPTION, "Unknown Exception")
    var error1 error
    error1, err = error0.Read(iprot)
    if err != nil {
      return
    }
    if err = iprot.ReadMessageEnd(); err != nil {
      return
    }
    err = error1
    return
  }
  if mTypeId != thrift.REPLY {
    err = thrift.NewApplicationException(thrift.INVALID_MESSAGE_TYPE_EXCEPTION, "do_root failed: invalid message type")
    return
  }
  result := MyRootDoRootResult{}
  if err = result.Read(iprot); err != nil {
    return
  }
  if err = iprot.ReadMessageEnd(); err != nil {
    return
  }
  return
}


type MyRootThreadsafeClient struct {
  Transport thrift.Transport
  ProtocolFactory thrift.ProtocolFactory
  InputProtocol thrift.Protocol
  OutputProtocol thrift.Protocol
  SeqId int32
  Mu sync.Mutex
}

func NewMyRootThreadsafeClientFactory(t thrift.Transport, f thrift.ProtocolFactory) *MyRootThreadsafeClient {
  return &MyRootThreadsafeClient{Transport: t,
    ProtocolFactory: f,
    InputProtocol: f.GetProtocol(t),
    OutputProtocol: f.GetProtocol(t),
    SeqId: 0,
  }
}

func NewMyRootThreadsafeClient(t thrift.Transport, iprot thrift.Protocol, oprot thrift.Protocol) *MyRootThreadsafeClient {
  return &MyRootThreadsafeClient{Transport: t,
    ProtocolFactory: nil,
    InputProtocol: iprot,
    OutputProtocol: oprot,
    SeqId: 0,
  }
}

func (p *MyRootThreadsafeClient) Threadsafe() {}

func (p *MyRootThreadsafeClient) DoRoot() (err error) {
  p.Mu.Lock()
  defer p.Mu.Unlock()
  if err = p.sendDoRoot(); err != nil { return }
  return p.recvDoRoot()
}

func (p *MyRootThreadsafeClient) sendDoRoot()(err error) {
  oprot := p.OutputProtocol
  if oprot == nil {
    oprot = p.ProtocolFactory.GetProtocol(p.Transport)
    p.OutputProtocol = oprot
  }
  p.SeqId++
  if err = oprot.WriteMessageBegin("do_root", thrift.CALL, p.SeqId); err != nil {
      return
  }
  args := MyRootDoRootArgs{
  }
  if err = args.Write(oprot); err != nil {
      return
  }
  if err = oprot.WriteMessageEnd(); err != nil {
      return
  }
  return oprot.Flush()
}


func (p *MyRootThreadsafeClient) recvDoRoot() (err error) {
  iprot := p.InputProtocol
  if iprot == nil {
    iprot = p.ProtocolFactory.GetProtocol(p.Transport)
    p.InputProtocol = iprot
  }
  method, mTypeId, seqId, err := iprot.ReadMessageBegin()
  if err != nil {
    return
  }
  if method != "do_root" {
    err = thrift.NewApplicationException(thrift.WRONG_METHOD_NAME, "do_root failed: wrong method name")
    return
  }
  if p.SeqId != seqId {
    err = thrift.NewApplicationException(thrift.BAD_SEQUENCE_ID, "do_root failed: out of sequence response")
    return
  }
  if mTypeId == thrift.EXCEPTION {
    error2 := thrift.NewApplicationException(thrift.UNKNOWN_APPLICATION_EXCEPTION, "Unknown Exception")
    var error3 error
    error3, err = error2.Read(iprot)
    if err != nil {
      return
    }
    if err = iprot.ReadMessageEnd(); err != nil {
      return
    }
    err = error3
    return
  }
  if mTypeId != thrift.REPLY {
    err = thrift.NewApplicationException(thrift.INVALID_MESSAGE_TYPE_EXCEPTION, "do_root failed: invalid message type")
    return
  }
  result := MyRootDoRootResult{}
  if err = result.Read(iprot); err != nil {
    return
  }
  if err = iprot.ReadMessageEnd(); err != nil {
    return
  }
  return
}


type MyRootProcessor struct {
  processorMap map[string]thrift.ProcessorFunction
  handler MyRoot
}

func (p *MyRootProcessor) AddToProcessorMap(key string, processor thrift.ProcessorFunction) {
  p.processorMap[key] = processor
}

func (p *MyRootProcessor) GetProcessorFunction(key string) (processor thrift.ProcessorFunction, err error) {
  if processor, ok := p.processorMap[key]; ok {
    return processor, nil
  }
  return nil, nil // generic error message will be sent
}

func (p *MyRootProcessor) ProcessorMap() map[string]thrift.ProcessorFunction {
  return p.processorMap
}

func NewMyRootProcessor(handler MyRoot) *MyRootProcessor {
  self4 := &MyRootProcessor{handler:handler, processorMap:make(map[string]thrift.ProcessorFunction)}
  self4.processorMap["do_root"] = &myRootProcessorDoRoot{handler:handler}
  return self4
}

type myRootProcessorDoRoot struct {
  handler MyRoot
}

func (p *myRootProcessorDoRoot) Read(iprot thrift.Protocol) (thrift.Struct, thrift.Exception) {
  args := MyRootDoRootArgs{}
  if err := args.Read(iprot); err != nil {
    return nil, err
  }
  iprot.ReadMessageEnd()
  return &args, nil
}

func (p *myRootProcessorDoRoot) Write(seqId int32, result thrift.WritableStruct, oprot thrift.Protocol) (err thrift.Exception) {
  var err2 error
  messageType := thrift.REPLY
  switch result.(type) {
  case thrift.ApplicationException:
    messageType = thrift.EXCEPTION
  }
  if err2 = oprot.WriteMessageBegin("do_root", messageType, seqId); err2 != nil {
    err = err2
  }
  if err2 = result.Write(oprot); err == nil && err2 != nil {
    err = err2
  }
  if err2 = oprot.WriteMessageEnd(); err == nil && err2 != nil {
    err = err2
  }
  if err2 = oprot.Flush(); err == nil && err2 != nil {
    err = err2
  }
  return err
}

func (p *myRootProcessorDoRoot) Run(argStruct thrift.Struct) (thrift.WritableStruct, thrift.ApplicationException) {
  var result MyRootDoRootResult
  if err := p.handler.DoRoot(); err != nil {
    switch err.(type) {
    default:
      x := thrift.NewApplicationException(thrift.INTERNAL_ERROR, "Internal error processing do_root: " + err.Error())
      return x, x
    }
  }
  return &result, nil
}


// HELPER FUNCTIONS AND STRUCTURES

type MyRootDoRootArgs struct {
}

func NewMyRootDoRootArgs() *MyRootDoRootArgs {
  return &MyRootDoRootArgs{}
}

func (p *MyRootDoRootArgs) Read(iprot thrift.Protocol) error {
  if _, err := iprot.ReadStructBegin(); err != nil {
    return thrift.PrependError(fmt.Sprintf("%T read error: ", p), err)
  }


  for {
    _, fieldTypeId, fieldId, err := iprot.ReadFieldBegin()
    if err != nil {
      return thrift.PrependError(fmt.Sprintf("%T field %d read error: ", p, fieldId), err)
    }
    if fieldTypeId == thrift.STOP { break; }
    if err := iprot.Skip(fieldTypeId); err != nil {
      return err
    }
    if err := iprot.ReadFieldEnd(); err != nil {
      return err
    }
  }
  if err := iprot.ReadStructEnd(); err != nil {
    return thrift.PrependError(fmt.Sprintf("%T read struct end error: ", p), err)
  }
  return nil
}

func (p *MyRootDoRootArgs) Write(oprot thrift.Protocol) error {
  if err := oprot.WriteStructBegin("do_root_args"); err != nil {
    return thrift.PrependError(fmt.Sprintf("%T write struct begin error: ", p), err) }
  if err := oprot.WriteFieldStop(); err != nil {
    return thrift.PrependError("write field stop error: ", err) }
  if err := oprot.WriteStructEnd(); err != nil {
    return thrift.PrependError("write struct stop error: ", err) }
  return nil
}

func (p *MyRootDoRootArgs) String() string {
  if p == nil {
    return "<nil>"
  }
  return fmt.Sprintf("MyRootDoRootArgs(%+v)", *p)
}

type MyRootDoRootResult struct {
}

func NewMyRootDoRootResult() *MyRootDoRootResult {
  return &MyRootDoRootResult{}
}

func (p *MyRootDoRootResult) Read(iprot thrift.Protocol) error {
  if _, err := iprot.ReadStructBegin(); err != nil {
    return thrift.PrependError(fmt.Sprintf("%T read error: ", p), err)
  }


  for {
    _, fieldTypeId, fieldId, err := iprot.ReadFieldBegin()
    if err != nil {
      return thrift.PrependError(fmt.Sprintf("%T field %d read error: ", p, fieldId), err)
    }
    if fieldTypeId == thrift.STOP { break; }
    if err := iprot.Skip(fieldTypeId); err != nil {
      return err
    }
    if err := iprot.ReadFieldEnd(); err != nil {
      return err
    }
  }
  if err := iprot.ReadStructEnd(); err != nil {
    return thrift.PrependError(fmt.Sprintf("%T read struct end error: ", p), err)
  }
  return nil
}

func (p *MyRootDoRootResult) Write(oprot thrift.Protocol) error {
  if err := oprot.WriteStructBegin("do_root_result"); err != nil {
    return thrift.PrependError(fmt.Sprintf("%T write struct begin error: ", p), err) }
  if err := oprot.WriteFieldStop(); err != nil {
    return thrift.PrependError("write field stop error: ", err) }
  if err := oprot.WriteStructEnd(); err != nil {
    return thrift.PrependError("write struct stop error: ", err) }
  return nil
}

func (p *MyRootDoRootResult) String() string {
  if p == nil {
    return "<nil>"
  }
  return fmt.Sprintf("MyRootDoRootResult(%+v)", *p)
}


