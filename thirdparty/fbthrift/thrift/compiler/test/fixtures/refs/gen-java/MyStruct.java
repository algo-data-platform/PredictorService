/**
 * Autogenerated by Thrift
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
import java.util.List;
import java.util.ArrayList;
import java.util.Map;
import java.util.HashMap;
import java.util.Set;
import java.util.HashSet;
import java.util.Collections;
import java.util.BitSet;
import java.util.Arrays;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.facebook.thrift.*;
import com.facebook.thrift.async.*;
import com.facebook.thrift.meta_data.*;
import com.facebook.thrift.server.*;
import com.facebook.thrift.transport.*;
import com.facebook.thrift.protocol.*;

@SuppressWarnings({ "unused", "serial" })
public class MyStruct implements TBase, java.io.Serializable, Cloneable, Comparable<MyStruct> {
  private static final TStruct STRUCT_DESC = new TStruct("MyStruct");
  private static final TField OPT_REF_FIELD_DESC = new TField("opt_ref", TType.STRUCT, (short)1);
  private static final TField REF_FIELD_DESC = new TField("ref", TType.STRUCT, (short)2);
  private static final TField REQ_REF_FIELD_DESC = new TField("req_ref", TType.STRUCT, (short)3);

  public MyField opt_ref;
  public MyField ref;
  public MyField req_ref;
  public static final int OPT_REF = 1;
  public static final int REF = 2;
  public static final int REQ_REF = 3;
  public static boolean DEFAULT_PRETTY_PRINT = true;

  // isset id assignments

  public static final Map<Integer, FieldMetaData> metaDataMap;
  static {
    Map<Integer, FieldMetaData> tmpMetaDataMap = new HashMap<Integer, FieldMetaData>();
    tmpMetaDataMap.put(OPT_REF, new FieldMetaData("opt_ref", TFieldRequirementType.OPTIONAL, 
        new StructMetaData(TType.STRUCT, MyField.class)));
    tmpMetaDataMap.put(REF, new FieldMetaData("ref", TFieldRequirementType.DEFAULT, 
        new StructMetaData(TType.STRUCT, MyField.class)));
    tmpMetaDataMap.put(REQ_REF, new FieldMetaData("req_ref", TFieldRequirementType.REQUIRED, 
        new StructMetaData(TType.STRUCT, MyField.class)));
    metaDataMap = Collections.unmodifiableMap(tmpMetaDataMap);
  }

  static {
    FieldMetaData.addStructMetaDataMap(MyStruct.class, metaDataMap);
  }

  public MyStruct() {
  }

  public MyStruct(
    MyField req_ref)
  {
    this();
    this.req_ref = req_ref;
  }

  public MyStruct(
    MyField ref,
    MyField req_ref)
  {
    this();
    this.ref = ref;
    this.req_ref = req_ref;
  }

  public MyStruct(
    MyField opt_ref,
    MyField ref,
    MyField req_ref)
  {
    this();
    this.opt_ref = opt_ref;
    this.ref = ref;
    this.req_ref = req_ref;
  }

  /**
   * Performs a deep copy on <i>other</i>.
   */
  public MyStruct(MyStruct other) {
    if (other.isSetOpt_ref()) {
      this.opt_ref = TBaseHelper.deepCopy(other.opt_ref);
    }
    if (other.isSetRef()) {
      this.ref = TBaseHelper.deepCopy(other.ref);
    }
    if (other.isSetReq_ref()) {
      this.req_ref = TBaseHelper.deepCopy(other.req_ref);
    }
  }

  public MyStruct deepCopy() {
    return new MyStruct(this);
  }

  @Deprecated
  public MyStruct clone() {
    return new MyStruct(this);
  }

  public MyField  getOpt_ref() {
    return this.opt_ref;
  }

  public MyStruct setOpt_ref(MyField opt_ref) {
    this.opt_ref = opt_ref;
    return this;
  }

  public void unsetOpt_ref() {
    this.opt_ref = null;
  }

  // Returns true if field opt_ref is set (has been assigned a value) and false otherwise
  public boolean isSetOpt_ref() {
    return this.opt_ref != null;
  }

  public void setOpt_refIsSet(boolean value) {
    if (!value) {
      this.opt_ref = null;
    }
  }

  public MyField  getRef() {
    return this.ref;
  }

  public MyStruct setRef(MyField ref) {
    this.ref = ref;
    return this;
  }

  public void unsetRef() {
    this.ref = null;
  }

  // Returns true if field ref is set (has been assigned a value) and false otherwise
  public boolean isSetRef() {
    return this.ref != null;
  }

  public void setRefIsSet(boolean value) {
    if (!value) {
      this.ref = null;
    }
  }

  public MyField  getReq_ref() {
    return this.req_ref;
  }

  public MyStruct setReq_ref(MyField req_ref) {
    this.req_ref = req_ref;
    return this;
  }

  public void unsetReq_ref() {
    this.req_ref = null;
  }

  // Returns true if field req_ref is set (has been assigned a value) and false otherwise
  public boolean isSetReq_ref() {
    return this.req_ref != null;
  }

  public void setReq_refIsSet(boolean value) {
    if (!value) {
      this.req_ref = null;
    }
  }

  public void setFieldValue(int fieldID, Object value) {
    switch (fieldID) {
    case OPT_REF:
      if (value == null) {
        unsetOpt_ref();
      } else {
        setOpt_ref((MyField)value);
      }
      break;

    case REF:
      if (value == null) {
        unsetRef();
      } else {
        setRef((MyField)value);
      }
      break;

    case REQ_REF:
      if (value == null) {
        unsetReq_ref();
      } else {
        setReq_ref((MyField)value);
      }
      break;

    default:
      throw new IllegalArgumentException("Field " + fieldID + " doesn't exist!");
    }
  }

  public Object getFieldValue(int fieldID) {
    switch (fieldID) {
    case OPT_REF:
      return getOpt_ref();

    case REF:
      return getRef();

    case REQ_REF:
      return getReq_ref();

    default:
      throw new IllegalArgumentException("Field " + fieldID + " doesn't exist!");
    }
  }

  // Returns true if field corresponding to fieldID is set (has been assigned a value) and false otherwise
  public boolean isSet(int fieldID) {
    switch (fieldID) {
    case OPT_REF:
      return isSetOpt_ref();
    case REF:
      return isSetRef();
    case REQ_REF:
      return isSetReq_ref();
    default:
      throw new IllegalArgumentException("Field " + fieldID + " doesn't exist!");
    }
  }

  @Override
  public boolean equals(Object that) {
    if (that == null)
      return false;
    if (that instanceof MyStruct)
      return this.equals((MyStruct)that);
    return false;
  }

  public boolean equals(MyStruct that) {
    if (that == null)
      return false;
    if (this == that)
      return true;

    boolean this_present_opt_ref = true && this.isSetOpt_ref();
    boolean that_present_opt_ref = true && that.isSetOpt_ref();
    if (this_present_opt_ref || that_present_opt_ref) {
      if (!(this_present_opt_ref && that_present_opt_ref))
        return false;
      if (!TBaseHelper.equalsNobinary(this.opt_ref, that.opt_ref))
        return false;
    }

    boolean this_present_ref = true && this.isSetRef();
    boolean that_present_ref = true && that.isSetRef();
    if (this_present_ref || that_present_ref) {
      if (!(this_present_ref && that_present_ref))
        return false;
      if (!TBaseHelper.equalsNobinary(this.ref, that.ref))
        return false;
    }

    boolean this_present_req_ref = true && this.isSetReq_ref();
    boolean that_present_req_ref = true && that.isSetReq_ref();
    if (this_present_req_ref || that_present_req_ref) {
      if (!(this_present_req_ref && that_present_req_ref))
        return false;
      if (!TBaseHelper.equalsNobinary(this.req_ref, that.req_ref))
        return false;
    }

    return true;
  }

  @Override
  public int hashCode() {
    return 0;
  }

  @Override
  public int compareTo(MyStruct other) {
    if (other == null) {
      // See java.lang.Comparable docs
      throw new NullPointerException();
    }

    if (other == this) {
      return 0;
    }
    int lastComparison = 0;

    lastComparison = Boolean.valueOf(isSetOpt_ref()).compareTo(other.isSetOpt_ref());
    if (lastComparison != 0) {
      return lastComparison;
    }
    lastComparison = TBaseHelper.compareTo(opt_ref, other.opt_ref);
    if (lastComparison != 0) {
      return lastComparison;
    }
    lastComparison = Boolean.valueOf(isSetRef()).compareTo(other.isSetRef());
    if (lastComparison != 0) {
      return lastComparison;
    }
    lastComparison = TBaseHelper.compareTo(ref, other.ref);
    if (lastComparison != 0) {
      return lastComparison;
    }
    lastComparison = Boolean.valueOf(isSetReq_ref()).compareTo(other.isSetReq_ref());
    if (lastComparison != 0) {
      return lastComparison;
    }
    lastComparison = TBaseHelper.compareTo(req_ref, other.req_ref);
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
        case OPT_REF:
          if (field.type == TType.STRUCT) {
            this.opt_ref = new MyField();
            this.opt_ref.read(iprot);
          } else { 
            TProtocolUtil.skip(iprot, field.type);
          }
          break;
        case REF:
          if (field.type == TType.STRUCT) {
            this.ref = new MyField();
            this.ref.read(iprot);
          } else { 
            TProtocolUtil.skip(iprot, field.type);
          }
          break;
        case REQ_REF:
          if (field.type == TType.STRUCT) {
            this.req_ref = new MyField();
            this.req_ref.read(iprot);
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
    if (this.opt_ref != null) {
      if (isSetOpt_ref()) {
        oprot.writeFieldBegin(OPT_REF_FIELD_DESC);
        this.opt_ref.write(oprot);
        oprot.writeFieldEnd();
      }
    }
    if (this.ref != null) {
      oprot.writeFieldBegin(REF_FIELD_DESC);
      this.ref.write(oprot);
      oprot.writeFieldEnd();
    }
    if (this.req_ref != null) {
      oprot.writeFieldBegin(REQ_REF_FIELD_DESC);
      this.req_ref.write(oprot);
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
    StringBuilder sb = new StringBuilder("MyStruct");
    sb.append(space);
    sb.append("(");
    sb.append(newLine);
    boolean first = true;

    if (isSetOpt_ref())
    {
      sb.append(indentStr);
      sb.append("opt_ref");
      sb.append(space);
      sb.append(":").append(space);
      if (this. getOpt_ref() == null) {
        sb.append("null");
      } else {
        sb.append(TBaseHelper.toString(this. getOpt_ref(), indent + 1, prettyPrint));
      }
      first = false;
    }
    if (!first) sb.append("," + newLine);
    sb.append(indentStr);
    sb.append("ref");
    sb.append(space);
    sb.append(":").append(space);
    if (this. getRef() == null) {
      sb.append("null");
    } else {
      sb.append(TBaseHelper.toString(this. getRef(), indent + 1, prettyPrint));
    }
    first = false;
    if (!first) sb.append("," + newLine);
    sb.append(indentStr);
    sb.append("req_ref");
    sb.append(space);
    sb.append(":").append(space);
    if (this. getReq_ref() == null) {
      sb.append("null");
    } else {
      sb.append(TBaseHelper.toString(this. getReq_ref(), indent + 1, prettyPrint));
    }
    first = false;
    sb.append(newLine + TBaseHelper.reduceIndent(indentStr));
    sb.append(")");
    return sb.toString();
  }

  public void validate() throws TException {
    // check for required fields
    if (req_ref == null) {
      throw new TProtocolException(TProtocolException.MISSING_REQUIRED_FIELD, "Required field 'req_ref' was not present! Struct: " + toString());
    }
    // check that fields of type enum have valid values
  }

}

