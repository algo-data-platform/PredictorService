/**
 * Autogenerated by Thrift
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
package com.facebook.thrift.direct_server;

import java.util.Map;
import java.util.HashMap;
import java.util.Collections;
import java.util.BitSet;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.facebook.thrift.*;
import com.facebook.thrift.async.*;
import com.facebook.thrift.meta_data.*;
import com.facebook.thrift.server.*;
import com.facebook.thrift.transport.*;
import com.facebook.thrift.protocol.*;

@SuppressWarnings({ "unused", "serial" })
public class JavaSimpleService {

  public interface Iface extends com.facebook.fbcode.fb303.FacebookService.Iface {

    public String simple(SimpleRequest r) throws TException;

    public String getString(int size) throws TException;

  }

  public interface AsyncIface extends com.facebook.fbcode.fb303.FacebookService.AsyncIface {

    public void simple(SimpleRequest r, AsyncMethodCallback resultHandler) throws TException;

    public void getString(int size, AsyncMethodCallback resultHandler) throws TException;

  }

  public static class Client extends com.facebook.fbcode.fb303.FacebookService.Client implements Iface, TClientIf {
    public Client(TProtocol prot)
    {
      this(prot, prot);
    }

    public Client(TProtocol iprot, TProtocol oprot)
    {
      super(iprot, oprot);
    }

    public String simple(SimpleRequest r) throws TException
    {
      ContextStack ctx = getContextStack("JavaSimpleService.simple", null);
      this.setContextStack(ctx);
      send_simple(r);
      return recv_simple();
    }

    public void send_simple(SimpleRequest r) throws TException
    {
      ContextStack ctx = this.getContextStack();
      super.preWrite(ctx, "JavaSimpleService.simple", null);
      oprot_.writeMessageBegin(new TMessage("simple", TMessageType.CALL, seqid_));
      simple_args args = new simple_args();
      args.r = r;
      args.write(oprot_);
      oprot_.writeMessageEnd();
      oprot_.getTransport().flush();
      super.postWrite(ctx, "JavaSimpleService.simple", args);
      return;
    }

    public String recv_simple() throws TException
    {
      ContextStack ctx = super.getContextStack();
      long bytes;
      TMessageType mtype;
      super.preRead(ctx, "JavaSimpleService.simple");
      TMessage msg = iprot_.readMessageBegin();
      if (msg.type == TMessageType.EXCEPTION) {
        TApplicationException x = TApplicationException.read(iprot_);
        iprot_.readMessageEnd();
        throw x;
      }
      simple_result result = new simple_result();
      result.read(iprot_);
      iprot_.readMessageEnd();
      super.postRead(ctx, "JavaSimpleService.simple", result);

      if (result.isSetSuccess()) {
        return result.success;
      }
      throw new TApplicationException(TApplicationException.MISSING_RESULT, "simple failed: unknown result");
    }

    public String getString(int size) throws TException
    {
      ContextStack ctx = getContextStack("JavaSimpleService.getString", null);
      this.setContextStack(ctx);
      send_getString(size);
      return recv_getString();
    }

    public void send_getString(int size) throws TException
    {
      ContextStack ctx = this.getContextStack();
      super.preWrite(ctx, "JavaSimpleService.getString", null);
      oprot_.writeMessageBegin(new TMessage("getString", TMessageType.CALL, seqid_));
      getString_args args = new getString_args();
      args.size = size;
      args.write(oprot_);
      oprot_.writeMessageEnd();
      oprot_.getTransport().flush();
      super.postWrite(ctx, "JavaSimpleService.getString", args);
      return;
    }

    public String recv_getString() throws TException
    {
      ContextStack ctx = super.getContextStack();
      long bytes;
      TMessageType mtype;
      super.preRead(ctx, "JavaSimpleService.getString");
      TMessage msg = iprot_.readMessageBegin();
      if (msg.type == TMessageType.EXCEPTION) {
        TApplicationException x = TApplicationException.read(iprot_);
        iprot_.readMessageEnd();
        throw x;
      }
      getString_result result = new getString_result();
      result.read(iprot_);
      iprot_.readMessageEnd();
      super.postRead(ctx, "JavaSimpleService.getString", result);

      if (result.isSetSuccess()) {
        return result.success;
      }
      throw new TApplicationException(TApplicationException.MISSING_RESULT, "getString failed: unknown result");
    }

  }
  public static class AsyncClient extends com.facebook.fbcode.fb303.FacebookService.AsyncClient implements AsyncIface {
    public static class Factory implements TAsyncClientFactory<AsyncClient> {
      private TAsyncClientManager clientManager;
      private TProtocolFactory protocolFactory;
      public Factory(TAsyncClientManager clientManager, TProtocolFactory protocolFactory) {
        this.clientManager = clientManager;
        this.protocolFactory = protocolFactory;
      }
      public AsyncClient getAsyncClient(TNonblockingTransport transport) {
        return new AsyncClient(protocolFactory, clientManager, transport);
      }
    }

    public AsyncClient(TProtocolFactory protocolFactory, TAsyncClientManager clientManager, TNonblockingTransport transport) {
      super(protocolFactory, clientManager, transport);
    }

    public void simple(SimpleRequest r, AsyncMethodCallback resultHandler5) throws TException {
      checkReady();
      simple_call method_call = new simple_call(r, resultHandler5, this, ___protocolFactory, ___transport);
      this.___currentMethod = method_call;
      ___manager.call(method_call);
    }

    public static class simple_call extends TAsyncMethodCall {
      private SimpleRequest r;
      public simple_call(SimpleRequest r, AsyncMethodCallback resultHandler6, TAsyncClient client2, TProtocolFactory protocolFactory3, TNonblockingTransport transport4) throws TException {
        super(client2, protocolFactory3, transport4, resultHandler6, false);
        this.r = r;
      }

      public void write_args(TProtocol prot) throws TException {
        prot.writeMessageBegin(new TMessage("simple", TMessageType.CALL, 0));
        simple_args args = new simple_args();
        args.setR(r);
        args.write(prot);
        prot.writeMessageEnd();
      }

      public String getResult() throws TException {
        if (getState() != State.RESPONSE_READ) {
          throw new IllegalStateException("Method call not finished!");
        }
        TMemoryInputTransport memoryTransport = new TMemoryInputTransport(getFrameBuffer().array());
        TProtocol prot = super.client.getProtocolFactory().getProtocol(memoryTransport);
        return (new Client(prot)).recv_simple();
      }
    }

    public void getString(int size, AsyncMethodCallback resultHandler10) throws TException {
      checkReady();
      getString_call method_call = new getString_call(size, resultHandler10, this, ___protocolFactory, ___transport);
      this.___currentMethod = method_call;
      ___manager.call(method_call);
    }

    public static class getString_call extends TAsyncMethodCall {
      private int size;
      public getString_call(int size, AsyncMethodCallback resultHandler11, TAsyncClient client7, TProtocolFactory protocolFactory8, TNonblockingTransport transport9) throws TException {
        super(client7, protocolFactory8, transport9, resultHandler11, false);
        this.size = size;
      }

      public void write_args(TProtocol prot) throws TException {
        prot.writeMessageBegin(new TMessage("getString", TMessageType.CALL, 0));
        getString_args args = new getString_args();
        args.setSize(size);
        args.write(prot);
        prot.writeMessageEnd();
      }

      public String getResult() throws TException {
        if (getState() != State.RESPONSE_READ) {
          throw new IllegalStateException("Method call not finished!");
        }
        TMemoryInputTransport memoryTransport = new TMemoryInputTransport(getFrameBuffer().array());
        TProtocol prot = super.client.getProtocolFactory().getProtocol(memoryTransport);
        return (new Client(prot)).recv_getString();
      }
    }

  }

  public static class Processor extends com.facebook.fbcode.fb303.FacebookService.Processor implements TProcessor {
    private static final Logger LOGGER = LoggerFactory.getLogger(Processor.class.getName());
    public Processor(Iface iface)
    {
      super(iface);
      iface_ = iface;
      processMap_.put("simple", new simple());
      processMap_.put("getString", new getString());
    }

    private Iface iface_;

    public boolean process(TProtocol iprot, TProtocol oprot, TConnectionContext server_ctx) throws TException
    {
      TMessage msg = iprot.readMessageBegin();
      ProcessFunction fn = processMap_.get(msg.name);
      if (fn == null) {
        TProtocolUtil.skip(iprot, TType.STRUCT);
        iprot.readMessageEnd();
        TApplicationException x = new TApplicationException(TApplicationException.UNKNOWN_METHOD, "Invalid method name: '"+msg.name+"'");
        oprot.writeMessageBegin(new TMessage(msg.name, TMessageType.EXCEPTION, msg.seqid));
        x.write(oprot);
        oprot.writeMessageEnd();
        oprot.getTransport().flush();
        return true;
      }
      fn.process(msg.seqid, iprot, oprot, server_ctx);
      return true;
    }

    private class simple implements ProcessFunction {
      public void process(int seqid, TProtocol iprot, TProtocol oprot, TConnectionContext server_ctx) throws TException
      {
        Object handler_ctx = event_handler_.getContext("JavaSimpleService.simple", server_ctx);
        simple_args args = new simple_args();
        event_handler_.preRead(handler_ctx, "JavaSimpleService.simple");
        args.read(iprot);
        iprot.readMessageEnd();
        event_handler_.postRead(handler_ctx, "JavaSimpleService.simple", args);
        simple_result result = new simple_result();
        result.success = iface_.simple(args.r);
        event_handler_.preWrite(handler_ctx, "JavaSimpleService.simple", result);
        oprot.writeMessageBegin(new TMessage("simple", TMessageType.REPLY, seqid));
        result.write(oprot);
        oprot.writeMessageEnd();
        oprot.getTransport().flush();
        event_handler_.postWrite(handler_ctx, "JavaSimpleService.simple", result);
      }

    }

    private class getString implements ProcessFunction {
      public void process(int seqid, TProtocol iprot, TProtocol oprot, TConnectionContext server_ctx) throws TException
      {
        Object handler_ctx = event_handler_.getContext("JavaSimpleService.getString", server_ctx);
        getString_args args = new getString_args();
        event_handler_.preRead(handler_ctx, "JavaSimpleService.getString");
        args.read(iprot);
        iprot.readMessageEnd();
        event_handler_.postRead(handler_ctx, "JavaSimpleService.getString", args);
        getString_result result = new getString_result();
        result.success = iface_.getString(args.size);
        event_handler_.preWrite(handler_ctx, "JavaSimpleService.getString", result);
        oprot.writeMessageBegin(new TMessage("getString", TMessageType.REPLY, seqid));
        result.write(oprot);
        oprot.writeMessageEnd();
        oprot.getTransport().flush();
        event_handler_.postWrite(handler_ctx, "JavaSimpleService.getString", result);
      }

    }

  }

  public static class simple_args implements TBase, java.io.Serializable, Cloneable, Comparable<simple_args>   {
    private static final TStruct STRUCT_DESC = new TStruct("simple_args");
    private static final TField R_FIELD_DESC = new TField("r", TType.STRUCT, (short)1);

    public SimpleRequest r;
    public static final int R = 1;
    public static boolean DEFAULT_PRETTY_PRINT = true;

    // isset id assignments

    public static final Map<Integer, FieldMetaData> metaDataMap;
    static {
      Map<Integer, FieldMetaData> tmpMetaDataMap = new HashMap<Integer, FieldMetaData>();
      tmpMetaDataMap.put(R, new FieldMetaData("r", TFieldRequirementType.DEFAULT, 
          new StructMetaData(TType.STRUCT, SimpleRequest.class)));
      metaDataMap = Collections.unmodifiableMap(tmpMetaDataMap);
    }

    static {
      FieldMetaData.addStructMetaDataMap(simple_args.class, metaDataMap);
    }

    public simple_args() {
    }

    public simple_args(
      SimpleRequest r)
    {
      this();
      this.r = r;
    }

    /**
     * Performs a deep copy on <i>other</i>.
     */
    public simple_args(simple_args other) {
      if (other.isSetR()) {
        this.r = TBaseHelper.deepCopy(other.r);
      }
    }

    public simple_args deepCopy() {
      return new simple_args(this);
    }

    @Deprecated
    public simple_args clone() {
      return new simple_args(this);
    }

    public SimpleRequest  getR() {
      return this.r;
    }

    public simple_args setR(SimpleRequest r) {
      this.r = r;
      return this;
    }

    public void unsetR() {
      this.r = null;
    }

    // Returns true if field r is set (has been assigned a value) and false otherwise
    public boolean isSetR() {
      return this.r != null;
    }

    public void setRIsSet(boolean value) {
      if (!value) {
        this.r = null;
      }
    }

    public void setFieldValue(int fieldID, Object value) {
      switch (fieldID) {
      case R:
        if (value == null) {
          unsetR();
        } else {
          setR((SimpleRequest)value);
        }
        break;

      default:
        throw new IllegalArgumentException("Field " + fieldID + " doesn't exist!");
      }
    }

    public Object getFieldValue(int fieldID) {
      switch (fieldID) {
      case R:
        return getR();

      default:
        throw new IllegalArgumentException("Field " + fieldID + " doesn't exist!");
      }
    }

    // Returns true if field corresponding to fieldID is set (has been assigned a value) and false otherwise
    public boolean isSet(int fieldID) {
      switch (fieldID) {
      case R:
        return isSetR();
      default:
        throw new IllegalArgumentException("Field " + fieldID + " doesn't exist!");
      }
    }

    @Override
    public boolean equals(Object that) {
      if (that == null)
        return false;
      if (that instanceof simple_args)
        return this.equals((simple_args)that);
      return false;
    }

    public boolean equals(simple_args that) {
      if (that == null)
        return false;
      if (this == that)
        return true;

      boolean this_present_r = true && this.isSetR();
      boolean that_present_r = true && that.isSetR();
      if (this_present_r || that_present_r) {
        if (!(this_present_r && that_present_r))
          return false;
        if (!TBaseHelper.equalsNobinary(this.r, that.r))
          return false;
      }

      return true;
    }

    @Override
    public int hashCode() {
      return 0;
    }

    @Override
    public int compareTo(simple_args other) {
      if (other == null) {
        // See java.lang.Comparable docs
        throw new NullPointerException();
      }

      if (other == this) {
        return 0;
      }
      int lastComparison = 0;

      lastComparison = Boolean.valueOf(isSetR()).compareTo(other.isSetR());
      if (lastComparison != 0) {
        return lastComparison;
      }
      lastComparison = TBaseHelper.compareTo(r, other.r);
      if (lastComparison != 0) {
        return lastComparison;
      }
      return 0;
    }

    public void read(TProtocol iprot) throws TException {
      TField field;
      iprot.readStructBegin(metaDataMap);
      while (true)
      {
        field = iprot.readFieldBegin();
        if (field.type == TType.STOP) { 
          break;
        }
        switch (field.id)
        {
          case R:
            if (field.type == TType.STRUCT) {
              this.r = new SimpleRequest();
              this.r.read(iprot);
            } else { 
              TProtocolUtil.skip(iprot, field.type);
            }
            break;
          default:
            TProtocolUtil.skip(iprot, field.type);
            break;
        }
        iprot.readFieldEnd();
      }
      iprot.readStructEnd();


      // check for required fields of primitive type, which can't be checked in the validate method
      validate();
    }

    public void write(TProtocol oprot) throws TException {
      validate();

      oprot.writeStructBegin(STRUCT_DESC);
      if (this.r != null) {
        oprot.writeFieldBegin(R_FIELD_DESC);
        this.r.write(oprot);
        oprot.writeFieldEnd();
      }
      oprot.writeFieldStop();
      oprot.writeStructEnd();
    }

    @Override
    public String toString() {
      return toString(DEFAULT_PRETTY_PRINT);
    }

    @Override
    public String toString(boolean prettyPrint) {
      return toString(1, prettyPrint);
    }

    @Override
    public String toString(int indent, boolean prettyPrint) {
      String indentStr = prettyPrint ? TBaseHelper.getIndentedString(indent) : "";
      String newLine = prettyPrint ? "\n" : "";
String space = prettyPrint ? " " : "";
      StringBuilder sb = new StringBuilder("simple_args");
      sb.append(space);
      sb.append("(");
      sb.append(newLine);
      boolean first = true;

      sb.append(indentStr);
      sb.append("r");
      sb.append(space);
      sb.append(":").append(space);
      if (this. getR() == null) {
        sb.append("null");
      } else {
        sb.append(TBaseHelper.toString(this. getR(), indent + 1, prettyPrint));
      }
      first = false;
      sb.append(newLine + TBaseHelper.reduceIndent(indentStr));
      sb.append(")");
      return sb.toString();
    }

    public void validate() throws TException {
      // check for required fields
      // check that fields of type enum have valid values
    }

  }

  public static class simple_result implements TBase, java.io.Serializable, Cloneable, Comparable<simple_result>   {
    private static final TStruct STRUCT_DESC = new TStruct("simple_result");
    private static final TField SUCCESS_FIELD_DESC = new TField("success", TType.STRING, (short)0);

    public String success;
    public static final int SUCCESS = 0;
    public static boolean DEFAULT_PRETTY_PRINT = true;

    // isset id assignments

    public static final Map<Integer, FieldMetaData> metaDataMap;
    static {
      Map<Integer, FieldMetaData> tmpMetaDataMap = new HashMap<Integer, FieldMetaData>();
      tmpMetaDataMap.put(SUCCESS, new FieldMetaData("success", TFieldRequirementType.DEFAULT, 
          new FieldValueMetaData(TType.STRING)));
      metaDataMap = Collections.unmodifiableMap(tmpMetaDataMap);
    }

    static {
      FieldMetaData.addStructMetaDataMap(simple_result.class, metaDataMap);
    }

    public simple_result() {
    }

    public simple_result(
      String success)
    {
      this();
      this.success = success;
    }

    /**
     * Performs a deep copy on <i>other</i>.
     */
    public simple_result(simple_result other) {
      if (other.isSetSuccess()) {
        this.success = TBaseHelper.deepCopy(other.success);
      }
    }

    public simple_result deepCopy() {
      return new simple_result(this);
    }

    @Deprecated
    public simple_result clone() {
      return new simple_result(this);
    }

    public String  getSuccess() {
      return this.success;
    }

    public simple_result setSuccess(String success) {
      this.success = success;
      return this;
    }

    public void unsetSuccess() {
      this.success = null;
    }

    // Returns true if field success is set (has been assigned a value) and false otherwise
    public boolean isSetSuccess() {
      return this.success != null;
    }

    public void setSuccessIsSet(boolean value) {
      if (!value) {
        this.success = null;
      }
    }

    public void setFieldValue(int fieldID, Object value) {
      switch (fieldID) {
      case SUCCESS:
        if (value == null) {
          unsetSuccess();
        } else {
          setSuccess((String)value);
        }
        break;

      default:
        throw new IllegalArgumentException("Field " + fieldID + " doesn't exist!");
      }
    }

    public Object getFieldValue(int fieldID) {
      switch (fieldID) {
      case SUCCESS:
        return getSuccess();

      default:
        throw new IllegalArgumentException("Field " + fieldID + " doesn't exist!");
      }
    }

    // Returns true if field corresponding to fieldID is set (has been assigned a value) and false otherwise
    public boolean isSet(int fieldID) {
      switch (fieldID) {
      case SUCCESS:
        return isSetSuccess();
      default:
        throw new IllegalArgumentException("Field " + fieldID + " doesn't exist!");
      }
    }

    @Override
    public boolean equals(Object that) {
      if (that == null)
        return false;
      if (that instanceof simple_result)
        return this.equals((simple_result)that);
      return false;
    }

    public boolean equals(simple_result that) {
      if (that == null)
        return false;
      if (this == that)
        return true;

      boolean this_present_success = true && this.isSetSuccess();
      boolean that_present_success = true && that.isSetSuccess();
      if (this_present_success || that_present_success) {
        if (!(this_present_success && that_present_success))
          return false;
        if (!TBaseHelper.equalsNobinary(this.success, that.success))
          return false;
      }

      return true;
    }

    @Override
    public int hashCode() {
      return 0;
    }

    @Override
    public int compareTo(simple_result other) {
      if (other == null) {
        // See java.lang.Comparable docs
        throw new NullPointerException();
      }

      if (other == this) {
        return 0;
      }
      int lastComparison = 0;

      lastComparison = Boolean.valueOf(isSetSuccess()).compareTo(other.isSetSuccess());
      if (lastComparison != 0) {
        return lastComparison;
      }
      lastComparison = TBaseHelper.compareTo(success, other.success);
      if (lastComparison != 0) {
        return lastComparison;
      }
      return 0;
    }

    public void read(TProtocol iprot) throws TException {
      TField field;
      iprot.readStructBegin(metaDataMap);
      while (true)
      {
        field = iprot.readFieldBegin();
        if (field.type == TType.STOP) { 
          break;
        }
        switch (field.id)
        {
          case SUCCESS:
            if (field.type == TType.STRING) {
              this.success = iprot.readString();
            } else { 
              TProtocolUtil.skip(iprot, field.type);
            }
            break;
          default:
            TProtocolUtil.skip(iprot, field.type);
            break;
        }
        iprot.readFieldEnd();
      }
      iprot.readStructEnd();


      // check for required fields of primitive type, which can't be checked in the validate method
      validate();
    }

    public void write(TProtocol oprot) throws TException {
      oprot.writeStructBegin(STRUCT_DESC);

      if (this.isSetSuccess()) {
        oprot.writeFieldBegin(SUCCESS_FIELD_DESC);
        oprot.writeString(this.success);
        oprot.writeFieldEnd();
      }
      oprot.writeFieldStop();
      oprot.writeStructEnd();
    }

    @Override
    public String toString() {
      return toString(DEFAULT_PRETTY_PRINT);
    }

    @Override
    public String toString(boolean prettyPrint) {
      return toString(1, prettyPrint);
    }

    @Override
    public String toString(int indent, boolean prettyPrint) {
      String indentStr = prettyPrint ? TBaseHelper.getIndentedString(indent) : "";
      String newLine = prettyPrint ? "\n" : "";
String space = prettyPrint ? " " : "";
      StringBuilder sb = new StringBuilder("simple_result");
      sb.append(space);
      sb.append("(");
      sb.append(newLine);
      boolean first = true;

      sb.append(indentStr);
      sb.append("success");
      sb.append(space);
      sb.append(":").append(space);
      if (this. getSuccess() == null) {
        sb.append("null");
      } else {
        sb.append(TBaseHelper.toString(this. getSuccess(), indent + 1, prettyPrint));
      }
      first = false;
      sb.append(newLine + TBaseHelper.reduceIndent(indentStr));
      sb.append(")");
      return sb.toString();
    }

    public void validate() throws TException {
      // check for required fields
      // check that fields of type enum have valid values
    }

  }

  public static class getString_args implements TBase, java.io.Serializable, Cloneable, Comparable<getString_args>   {
    private static final TStruct STRUCT_DESC = new TStruct("getString_args");
    private static final TField SIZE_FIELD_DESC = new TField("size", TType.I32, (short)1);

    public int size;
    public static final int SIZE = 1;
    public static boolean DEFAULT_PRETTY_PRINT = true;

    // isset id assignments
    private static final int __SIZE_ISSET_ID = 0;
    private BitSet __isset_bit_vector = new BitSet(1);

    public static final Map<Integer, FieldMetaData> metaDataMap;
    static {
      Map<Integer, FieldMetaData> tmpMetaDataMap = new HashMap<Integer, FieldMetaData>();
      tmpMetaDataMap.put(SIZE, new FieldMetaData("size", TFieldRequirementType.DEFAULT, 
          new FieldValueMetaData(TType.I32)));
      metaDataMap = Collections.unmodifiableMap(tmpMetaDataMap);
    }

    static {
      FieldMetaData.addStructMetaDataMap(getString_args.class, metaDataMap);
    }

    public getString_args() {
    }

    public getString_args(
      int size)
    {
      this();
      this.size = size;
      setSizeIsSet(true);
    }

    /**
     * Performs a deep copy on <i>other</i>.
     */
    public getString_args(getString_args other) {
      __isset_bit_vector.clear();
      __isset_bit_vector.or(other.__isset_bit_vector);
      this.size = TBaseHelper.deepCopy(other.size);
    }

    public getString_args deepCopy() {
      return new getString_args(this);
    }

    @Deprecated
    public getString_args clone() {
      return new getString_args(this);
    }

    public int  getSize() {
      return this.size;
    }

    public getString_args setSize(int size) {
      this.size = size;
      setSizeIsSet(true);
      return this;
    }

    public void unsetSize() {
      __isset_bit_vector.clear(__SIZE_ISSET_ID);
    }

    // Returns true if field size is set (has been assigned a value) and false otherwise
    public boolean isSetSize() {
      return __isset_bit_vector.get(__SIZE_ISSET_ID);
    }

    public void setSizeIsSet(boolean value) {
      __isset_bit_vector.set(__SIZE_ISSET_ID, value);
    }

    public void setFieldValue(int fieldID, Object value) {
      switch (fieldID) {
      case SIZE:
        if (value == null) {
          unsetSize();
        } else {
          setSize((Integer)value);
        }
        break;

      default:
        throw new IllegalArgumentException("Field " + fieldID + " doesn't exist!");
      }
    }

    public Object getFieldValue(int fieldID) {
      switch (fieldID) {
      case SIZE:
        return new Integer(getSize());

      default:
        throw new IllegalArgumentException("Field " + fieldID + " doesn't exist!");
      }
    }

    // Returns true if field corresponding to fieldID is set (has been assigned a value) and false otherwise
    public boolean isSet(int fieldID) {
      switch (fieldID) {
      case SIZE:
        return isSetSize();
      default:
        throw new IllegalArgumentException("Field " + fieldID + " doesn't exist!");
      }
    }

    @Override
    public boolean equals(Object that) {
      if (that == null)
        return false;
      if (that instanceof getString_args)
        return this.equals((getString_args)that);
      return false;
    }

    public boolean equals(getString_args that) {
      if (that == null)
        return false;
      if (this == that)
        return true;

      boolean this_present_size = true;
      boolean that_present_size = true;
      if (this_present_size || that_present_size) {
        if (!(this_present_size && that_present_size))
          return false;
        if (!TBaseHelper.equalsNobinary(this.size, that.size))
          return false;
      }

      return true;
    }

    @Override
    public int hashCode() {
      return 0;
    }

    @Override
    public int compareTo(getString_args other) {
      if (other == null) {
        // See java.lang.Comparable docs
        throw new NullPointerException();
      }

      if (other == this) {
        return 0;
      }
      int lastComparison = 0;

      lastComparison = Boolean.valueOf(isSetSize()).compareTo(other.isSetSize());
      if (lastComparison != 0) {
        return lastComparison;
      }
      lastComparison = TBaseHelper.compareTo(size, other.size);
      if (lastComparison != 0) {
        return lastComparison;
      }
      return 0;
    }

    public void read(TProtocol iprot) throws TException {
      TField field;
      iprot.readStructBegin(metaDataMap);
      while (true)
      {
        field = iprot.readFieldBegin();
        if (field.type == TType.STOP) { 
          break;
        }
        switch (field.id)
        {
          case SIZE:
            if (field.type == TType.I32) {
              this.size = iprot.readI32();
              setSizeIsSet(true);
            } else { 
              TProtocolUtil.skip(iprot, field.type);
            }
            break;
          default:
            TProtocolUtil.skip(iprot, field.type);
            break;
        }
        iprot.readFieldEnd();
      }
      iprot.readStructEnd();


      // check for required fields of primitive type, which can't be checked in the validate method
      validate();
    }

    public void write(TProtocol oprot) throws TException {
      validate();

      oprot.writeStructBegin(STRUCT_DESC);
      oprot.writeFieldBegin(SIZE_FIELD_DESC);
      oprot.writeI32(this.size);
      oprot.writeFieldEnd();
      oprot.writeFieldStop();
      oprot.writeStructEnd();
    }

    @Override
    public String toString() {
      return toString(DEFAULT_PRETTY_PRINT);
    }

    @Override
    public String toString(boolean prettyPrint) {
      return toString(1, prettyPrint);
    }

    @Override
    public String toString(int indent, boolean prettyPrint) {
      String indentStr = prettyPrint ? TBaseHelper.getIndentedString(indent) : "";
      String newLine = prettyPrint ? "\n" : "";
String space = prettyPrint ? " " : "";
      StringBuilder sb = new StringBuilder("getString_args");
      sb.append(space);
      sb.append("(");
      sb.append(newLine);
      boolean first = true;

      sb.append(indentStr);
      sb.append("size");
      sb.append(space);
      sb.append(":").append(space);
      sb.append(TBaseHelper.toString(this. getSize(), indent + 1, prettyPrint));
      first = false;
      sb.append(newLine + TBaseHelper.reduceIndent(indentStr));
      sb.append(")");
      return sb.toString();
    }

    public void validate() throws TException {
      // check for required fields
      // check that fields of type enum have valid values
    }

  }

  public static class getString_result implements TBase, java.io.Serializable, Cloneable, Comparable<getString_result>   {
    private static final TStruct STRUCT_DESC = new TStruct("getString_result");
    private static final TField SUCCESS_FIELD_DESC = new TField("success", TType.STRING, (short)0);

    public String success;
    public static final int SUCCESS = 0;
    public static boolean DEFAULT_PRETTY_PRINT = true;

    // isset id assignments

    public static final Map<Integer, FieldMetaData> metaDataMap;
    static {
      Map<Integer, FieldMetaData> tmpMetaDataMap = new HashMap<Integer, FieldMetaData>();
      tmpMetaDataMap.put(SUCCESS, new FieldMetaData("success", TFieldRequirementType.DEFAULT, 
          new FieldValueMetaData(TType.STRING)));
      metaDataMap = Collections.unmodifiableMap(tmpMetaDataMap);
    }

    static {
      FieldMetaData.addStructMetaDataMap(getString_result.class, metaDataMap);
    }

    public getString_result() {
    }

    public getString_result(
      String success)
    {
      this();
      this.success = success;
    }

    /**
     * Performs a deep copy on <i>other</i>.
     */
    public getString_result(getString_result other) {
      if (other.isSetSuccess()) {
        this.success = TBaseHelper.deepCopy(other.success);
      }
    }

    public getString_result deepCopy() {
      return new getString_result(this);
    }

    @Deprecated
    public getString_result clone() {
      return new getString_result(this);
    }

    public String  getSuccess() {
      return this.success;
    }

    public getString_result setSuccess(String success) {
      this.success = success;
      return this;
    }

    public void unsetSuccess() {
      this.success = null;
    }

    // Returns true if field success is set (has been assigned a value) and false otherwise
    public boolean isSetSuccess() {
      return this.success != null;
    }

    public void setSuccessIsSet(boolean value) {
      if (!value) {
        this.success = null;
      }
    }

    public void setFieldValue(int fieldID, Object value) {
      switch (fieldID) {
      case SUCCESS:
        if (value == null) {
          unsetSuccess();
        } else {
          setSuccess((String)value);
        }
        break;

      default:
        throw new IllegalArgumentException("Field " + fieldID + " doesn't exist!");
      }
    }

    public Object getFieldValue(int fieldID) {
      switch (fieldID) {
      case SUCCESS:
        return getSuccess();

      default:
        throw new IllegalArgumentException("Field " + fieldID + " doesn't exist!");
      }
    }

    // Returns true if field corresponding to fieldID is set (has been assigned a value) and false otherwise
    public boolean isSet(int fieldID) {
      switch (fieldID) {
      case SUCCESS:
        return isSetSuccess();
      default:
        throw new IllegalArgumentException("Field " + fieldID + " doesn't exist!");
      }
    }

    @Override
    public boolean equals(Object that) {
      if (that == null)
        return false;
      if (that instanceof getString_result)
        return this.equals((getString_result)that);
      return false;
    }

    public boolean equals(getString_result that) {
      if (that == null)
        return false;
      if (this == that)
        return true;

      boolean this_present_success = true && this.isSetSuccess();
      boolean that_present_success = true && that.isSetSuccess();
      if (this_present_success || that_present_success) {
        if (!(this_present_success && that_present_success))
          return false;
        if (!TBaseHelper.equalsNobinary(this.success, that.success))
          return false;
      }

      return true;
    }

    @Override
    public int hashCode() {
      return 0;
    }

    @Override
    public int compareTo(getString_result other) {
      if (other == null) {
        // See java.lang.Comparable docs
        throw new NullPointerException();
      }

      if (other == this) {
        return 0;
      }
      int lastComparison = 0;

      lastComparison = Boolean.valueOf(isSetSuccess()).compareTo(other.isSetSuccess());
      if (lastComparison != 0) {
        return lastComparison;
      }
      lastComparison = TBaseHelper.compareTo(success, other.success);
      if (lastComparison != 0) {
        return lastComparison;
      }
      return 0;
    }

    public void read(TProtocol iprot) throws TException {
      TField field;
      iprot.readStructBegin(metaDataMap);
      while (true)
      {
        field = iprot.readFieldBegin();
        if (field.type == TType.STOP) { 
          break;
        }
        switch (field.id)
        {
          case SUCCESS:
            if (field.type == TType.STRING) {
              this.success = iprot.readString();
            } else { 
              TProtocolUtil.skip(iprot, field.type);
            }
            break;
          default:
            TProtocolUtil.skip(iprot, field.type);
            break;
        }
        iprot.readFieldEnd();
      }
      iprot.readStructEnd();


      // check for required fields of primitive type, which can't be checked in the validate method
      validate();
    }

    public void write(TProtocol oprot) throws TException {
      oprot.writeStructBegin(STRUCT_DESC);

      if (this.isSetSuccess()) {
        oprot.writeFieldBegin(SUCCESS_FIELD_DESC);
        oprot.writeString(this.success);
        oprot.writeFieldEnd();
      }
      oprot.writeFieldStop();
      oprot.writeStructEnd();
    }

    @Override
    public String toString() {
      return toString(DEFAULT_PRETTY_PRINT);
    }

    @Override
    public String toString(boolean prettyPrint) {
      return toString(1, prettyPrint);
    }

    @Override
    public String toString(int indent, boolean prettyPrint) {
      String indentStr = prettyPrint ? TBaseHelper.getIndentedString(indent) : "";
      String newLine = prettyPrint ? "\n" : "";
String space = prettyPrint ? " " : "";
      StringBuilder sb = new StringBuilder("getString_result");
      sb.append(space);
      sb.append("(");
      sb.append(newLine);
      boolean first = true;

      sb.append(indentStr);
      sb.append("success");
      sb.append(space);
      sb.append(":").append(space);
      if (this. getSuccess() == null) {
        sb.append("null");
      } else {
        sb.append(TBaseHelper.toString(this. getSuccess(), indent + 1, prettyPrint));
      }
      first = false;
      sb.append(newLine + TBaseHelper.reduceIndent(indentStr));
      sb.append(")");
      return sb.toString();
    }

    public void validate() throws TException {
      // check for required fields
      // check that fields of type enum have valid values
    }

  }

}
