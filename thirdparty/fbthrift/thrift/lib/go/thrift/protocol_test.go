/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements. See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership. The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

package thrift

import (
	"bytes"
	"io/ioutil"
	"math"
	"net"
	"net/http"
	"testing"
)

const PROTOCOL_BINARY_DATA_SIZE = 155

var (
	data           string // test data for writing
	protocol_bdata []byte // test data for writing; same as data
	boolValues     = []bool{false, true, false, false, true}
	byteValues     = []byte{117, 0, 1, 32, 127, 128, 255}
	int16Values    = []int16{459, 0, 1, -1, -128, 127, 32767, -32768}
	int32Values    = []int32{459, 0, 1, -1, -128, 127, 32767, 2147483647, -2147483535}
	int64Values    = []int64{459, 0, 1, -1, -128, 127, 32767, 2147483647, -2147483535, 34359738481, -35184372088719, -9223372036854775808, 9223372036854775807}
	doubleValues   = []float64{459.3, 0.0, -1.0, 1.0, 0.5, 0.3333, 3.14159, 1.537e-38, 1.673e25, 6.02214179e23, -6.02214179e23, INFINITY.Float64(), NEGATIVE_INFINITY.Float64(), NAN.Float64()}
	floatValues    = []float32{459.3, 0.0, -1.0, 1.0, 0.5, 0.3333, 3.14159, 1.537e-38, 1.673e25, 6.02214179e23, -6.02214179e23, INFINITY.Float32(), NEGATIVE_INFINITY.Float32(), NAN.Float32()}
	stringValues   = []string{"", "a", "st[uf]f", "st,u:ff with spaces", "stuff\twith\nescape\\characters'...\"lots{of}fun</xml>"}
)

// var floatValues   []float32 = []float32{459.3, 0.0, -1.0, 1.0, 0.5, 0.3333, 3.14159, 1.537e-38, 1.673e25, 6.02214179e23, -6.02214179e23, INFINITY.Float32(), NEGATIVE_INFINITY.Float32(), NAN.Float32()}

// func floatValues() []
func init() {
	protocol_bdata = make([]byte, PROTOCOL_BINARY_DATA_SIZE)
	for i := 0; i < PROTOCOL_BINARY_DATA_SIZE; i++ {
		protocol_bdata[i] = byte((i + 'a') % 255)
	}
	data = string(protocol_bdata)
}

type HTTPEchoServer struct{}
type HTTPHeaderEchoServer struct{}

func (p *HTTPEchoServer) ServeHTTP(w http.ResponseWriter, req *http.Request) {
	buf, err := ioutil.ReadAll(req.Body)
	if err != nil {
		w.WriteHeader(http.StatusBadRequest)
		w.Write(buf)
	} else {
		w.WriteHeader(http.StatusOK)
		w.Write(buf)
	}
}

func (p *HTTPHeaderEchoServer) ServeHTTP(w http.ResponseWriter, req *http.Request) {
	buf, err := ioutil.ReadAll(req.Body)
	if err != nil {
		w.WriteHeader(http.StatusBadRequest)
		w.Write(buf)
	} else {
		w.WriteHeader(http.StatusOK)
		w.Write(buf)
	}
}

func HTTPClientSetupForTest(t *testing.T) net.Listener {
	l, err := net.Listen("tcp", "127.0.0.1:0")
	if err != nil {
		t.Fatalf("Unable to setup tcp listener on local port: %s", err)
		return l
	}
	go http.Serve(l, &HTTPEchoServer{})
	return l
}

func HTTPClientSetupForHeaderTest(t *testing.T) net.Listener {
	l, err := net.Listen("tcp", "127.0.0.1:0")
	if err != nil {
		t.Fatalf("Unable to setup tcp listener on local port: %s", err)
		return l
	}
	go http.Serve(l, &HTTPHeaderEchoServer{})
	return l
}

func ReadWriteProtocolTest(t *testing.T, protocolFactory ProtocolFactory) {
	buf := bytes.NewBuffer(make([]byte, 0, 1024))
	l := HTTPClientSetupForTest(t)
	defer l.Close()
	transports := []TransportFactory{
		NewMemoryBufferTransportFactory(1024),
		NewStreamTransportFactory(buf, buf, true),
		NewFramedTransportFactory(NewMemoryBufferTransportFactory(1024)),
		NewHTTPPostClientTransportFactory("http://" + l.Addr().String()),
	}
	for _, tf := range transports {
		trans := tf.GetTransport(nil)
		p := protocolFactory.GetProtocol(trans)
		ReadWriteBool(t, p, trans)
		trans.Close()
	}
	for _, tf := range transports {
		trans := tf.GetTransport(nil)
		p := protocolFactory.GetProtocol(trans)
		ReadWriteByte(t, p, trans)
		trans.Close()
	}
	for _, tf := range transports {
		trans := tf.GetTransport(nil)
		p := protocolFactory.GetProtocol(trans)
		ReadWriteI16(t, p, trans)
		trans.Close()
	}
	for _, tf := range transports {
		trans := tf.GetTransport(nil)
		p := protocolFactory.GetProtocol(trans)
		ReadWriteI32(t, p, trans)
		trans.Close()
	}
	for _, tf := range transports {
		trans := tf.GetTransport(nil)
		p := protocolFactory.GetProtocol(trans)
		ReadWriteI64(t, p, trans)
		trans.Close()
	}
	for _, tf := range transports {
		trans := tf.GetTransport(nil)
		p := protocolFactory.GetProtocol(trans)
		ReadWriteDouble(t, p, trans)
		trans.Close()
	}
	for _, tf := range transports {
		trans := tf.GetTransport(nil)
		p := protocolFactory.GetProtocol(trans)
		ReadWriteFloat(t, p, trans)
		trans.Close()
	}
	for _, tf := range transports {
		trans := tf.GetTransport(nil)
		p := protocolFactory.GetProtocol(trans)
		ReadWriteString(t, p, trans)
		trans.Close()
	}
	for _, tf := range transports {
		trans := tf.GetTransport(nil)
		p := protocolFactory.GetProtocol(trans)
		ReadWriteBinary(t, p, trans)
		trans.Close()
	}
	for _, tf := range transports {
		trans := tf.GetTransport(nil)
		p := protocolFactory.GetProtocol(trans)
		ReadWriteI64(t, p, trans)
		ReadWriteDouble(t, p, trans)
		ReadWriteFloat(t, p, trans)
		ReadWriteBinary(t, p, trans)
		ReadWriteByte(t, p, trans)
		trans.Close()
	}
}

func ReadWriteBool(t testing.TB, p Protocol, trans Transport) {
	thetype := Type(BOOL)
	thelen := len(boolValues)
	err := p.WriteListBegin(thetype, thelen)
	if err != nil {
		t.Errorf("%s: %T %T %q Error writing list begin: %q", "ReadWriteBool", p, trans, err, thetype)
	}
	for k, v := range boolValues {
		err = p.WriteBool(v)
		if err != nil {
			t.Errorf("%s: %T %T %q Error writing bool in list at index %d: %t", "ReadWriteBool", p, trans, err, k, v)
		}
	}
	p.WriteListEnd()
	if err != nil {
		t.Errorf("%s: %T %T %q Error writing list end: %v", "ReadWriteBool", p, trans, err, boolValues)
	}
	p.Flush()
	thetype2, thelen2, err := p.ReadListBegin()
	if err != nil {
		t.Errorf("%s: %T %T %q Error reading list: %v", "ReadWriteBool", p, trans, err, boolValues)
	}
	_, ok := p.(*SimpleJSONProtocol)
	if !ok {
		if thetype != thetype2 {
			t.Errorf("%s: %T %T type %s != type %s", "ReadWriteBool", p, trans, thetype, thetype2)
		}
		if thelen != thelen2 {
			t.Errorf("%s: %T %T len %d != len %d", "ReadWriteBool", p, trans, thelen, thelen2)
		}
	}
	for k, v := range boolValues {
		value, err := p.ReadBool()
		if err != nil {
			t.Errorf("%s: %T %T %q Error reading bool at index %d: %t", "ReadWriteBool", p, trans, err, k, v)
		}
		if v != value {
			t.Errorf("%s: index %d %q %q %t != %t", "ReadWriteBool", k, p, trans, v, value)
		}
	}
	err = p.ReadListEnd()
	if err != nil {
		t.Errorf("%s: %T %T Unable to read list end: %q", "ReadWriteBool", p, trans, err)
	}
}

func ReadWriteByte(t testing.TB, p Protocol, trans Transport) {
	thetype := Type(BYTE)
	thelen := len(byteValues)
	err := p.WriteListBegin(thetype, thelen)
	if err != nil {
		t.Errorf("%s: %T %T %q Error writing list begin: %q", "ReadWriteByte", p, trans, err, thetype)
	}
	for k, v := range byteValues {
		err = p.WriteByte(v)
		if err != nil {
			t.Errorf("%s: %T %T %q Error writing byte in list at index %d: %q", "ReadWriteByte", p, trans, err, k, v)
		}
	}
	err = p.WriteListEnd()
	if err != nil {
		t.Errorf("%s: %T %T %q Error writing list end: %q", "ReadWriteByte", p, trans, err, byteValues)
	}
	err = p.Flush()
	if err != nil {
		t.Errorf("%s: %T %T %q Error flushing list of bytes: %q", "ReadWriteByte", p, trans, err, byteValues)
	}
	thetype2, thelen2, err := p.ReadListBegin()
	if err != nil {
		t.Errorf("%s: %T %T %q Error reading list: %q", "ReadWriteByte", p, trans, err, byteValues)
	}
	_, ok := p.(*SimpleJSONProtocol)
	if !ok {
		if thetype != thetype2 {
			t.Errorf("%s: %T %T type %s != type %s", "ReadWriteByte", p, trans, thetype, thetype2)
		}
		if thelen != thelen2 {
			t.Errorf("%s: %T %T len %d != len %d", "ReadWriteByte", p, trans, thelen, thelen2)
		}
	}
	for k, v := range byteValues {
		value, err := p.ReadByte()
		if err != nil {
			t.Errorf("%s: %T %T %q Error reading byte at index %d: %q", "ReadWriteByte", p, trans, err, k, v)
		}
		if v != value {
			t.Errorf("%s: %T %T %d != %d", "ReadWriteByte", p, trans, v, value)
		}
	}
	err = p.ReadListEnd()
	if err != nil {
		t.Errorf("%s: %T %T Unable to read list end: %q", "ReadWriteByte", p, trans, err)
	}
}

func ReadWriteI16(t testing.TB, p Protocol, trans Transport) {
	thetype := Type(I16)
	thelen := len(int16Values)
	p.WriteListBegin(thetype, thelen)
	for _, v := range int16Values {
		p.WriteI16(v)
	}
	p.WriteListEnd()
	p.Flush()
	thetype2, thelen2, err := p.ReadListBegin()
	if err != nil {
		t.Errorf("%s: %T %T %q Error reading list: %q", "ReadWriteI16", p, trans, err, int16Values)
	}
	_, ok := p.(*SimpleJSONProtocol)
	if !ok {
		if thetype != thetype2 {
			t.Errorf("%s: %T %T type %s != type %s", "ReadWriteI16", p, trans, thetype, thetype2)
		}
		if thelen != thelen2 {
			t.Errorf("%s: %T %T len %d != len %d", "ReadWriteI16", p, trans, thelen, thelen2)
		}
	}
	for k, v := range int16Values {
		value, err := p.ReadI16()
		if err != nil {
			t.Errorf("%s: %T %T %q Error reading int16 at index %d: %q", "ReadWriteI16", p, trans, err, k, v)
		}
		if v != value {
			t.Errorf("%s: %T %T %d != %d", "ReadWriteI16", p, trans, v, value)
		}
	}
	err = p.ReadListEnd()
	if err != nil {
		t.Errorf("%s: %T %T Unable to read list end: %q", "ReadWriteI16", p, trans, err)
	}
}

func ReadWriteI32(t testing.TB, p Protocol, trans Transport) {
	thetype := Type(I32)
	thelen := len(int32Values)
	p.WriteListBegin(thetype, thelen)
	for _, v := range int32Values {
		p.WriteI32(v)
	}
	p.WriteListEnd()
	p.Flush()
	thetype2, thelen2, err := p.ReadListBegin()
	if err != nil {
		t.Errorf("%s: %T %T %q Error reading list: %q", "ReadWriteI32", p, trans, err, int32Values)
	}
	_, ok := p.(*SimpleJSONProtocol)
	if !ok {
		if thetype != thetype2 {
			t.Errorf("%s: %T %T type %s != type %s", "ReadWriteI32", p, trans, thetype, thetype2)
		}
		if thelen != thelen2 {
			t.Errorf("%s: %T %T len %d != len %d", "ReadWriteI32", p, trans, thelen, thelen2)
		}
	}
	for k, v := range int32Values {
		value, err := p.ReadI32()
		if err != nil {
			t.Errorf("%s: %T %T %q Error reading int32 at index %d: %q", "ReadWriteI32", p, trans, err, k, v)
		}
		if v != value {
			t.Errorf("%s: %T %T %d != %d", "ReadWriteI32", p, trans, v, value)
		}
	}
	if err != nil {
		t.Errorf("%s: %T %T Unable to read list end: %q", "ReadWriteI32", p, trans, err)
	}
}

func ReadWriteI64(t testing.TB, p Protocol, trans Transport) {
	thetype := Type(I64)
	thelen := len(int64Values)
	p.WriteListBegin(thetype, thelen)
	for _, v := range int64Values {
		p.WriteI64(v)
	}
	p.WriteListEnd()
	p.Flush()
	thetype2, thelen2, err := p.ReadListBegin()
	if err != nil {
		t.Errorf("%s: %T %T %q Error reading list: %q", "ReadWriteI64", p, trans, err, int64Values)
	}
	_, ok := p.(*SimpleJSONProtocol)
	if !ok {
		if thetype != thetype2 {
			t.Errorf("%s: %T %T type %s != type %s", "ReadWriteI64", p, trans, thetype, thetype2)
		}
		if thelen != thelen2 {
			t.Errorf("%s: %T %T len %d != len %d", "ReadWriteI64", p, trans, thelen, thelen2)
		}
	}
	for k, v := range int64Values {
		value, err := p.ReadI64()
		if err != nil {
			t.Errorf("%s: %T %T %q Error reading int64 at index %d: %q", "ReadWriteI64", p, trans, err, k, v)
		}
		if v != value {
			t.Errorf("%s: %T %T %q != %q", "ReadWriteI64", p, trans, v, value)
		}
	}
	if err != nil {
		t.Errorf("%s: %T %T Unable to read list end: %q", "ReadWriteI64", p, trans, err)
	}
}

func ReadWriteDouble(t testing.TB, p Protocol, trans Transport) {
	doubleValues = []float64{459.3, 0.0, -1.0, 1.0, 0.5, 0.3333, 3.14159, 1.537e-38, 1.673e25, 6.02214179e23, -6.02214179e23, INFINITY.Float64(), NEGATIVE_INFINITY.Float64(), NAN.Float64()}

	thetype := Type(DOUBLE)
	thelen := len(doubleValues)
	p.WriteListBegin(thetype, thelen)
	for _, v := range doubleValues {
		p.WriteDouble(v)
	}
	p.WriteListEnd()
	p.Flush()
	thetype2, thelen2, err := p.ReadListBegin()
	if err != nil {
		t.Errorf("%s: %T %T %q Error reading list: %v", "ReadWriteDouble", p, trans, err, doubleValues)
	}
	if thetype != thetype2 {
		t.Errorf("%s: %T %T type %s != type %s", "ReadWriteDouble", p, trans, thetype, thetype2)
	}
	if thelen != thelen2 {
		t.Errorf("%s: %T %T len %d != len %d", "ReadWriteDouble", p, trans, thelen, thelen2)
	}
	for k, v := range doubleValues {
		value, err := p.ReadDouble()
		if err != nil {
			t.Errorf("%s: %T %T %q Error reading double at index %d: %f", "ReadWriteDouble", p, trans, err, k, v)
		}
		if math.IsNaN(v) {
			if !math.IsNaN(value) {
				t.Errorf("%s: %T %T math.IsNaN(%f) != math.IsNaN(%f)", "ReadWriteDouble", p, trans, v, value)
			}
		} else if v != value {
			t.Errorf("%s: %T %T %f != %f", "ReadWriteDouble", p, trans, v, value)
		}
	}
	err = p.ReadListEnd()
	if err != nil {
		t.Errorf("%s: %T %T Unable to read list end: %q", "ReadWriteDouble", p, trans, err)
	}
}

func ReadWriteFloat(t testing.TB, p Protocol, trans Transport) {
	floatValues = []float32{459.3, 0.0, -1.0, 1.0, 0.5, 0.3333, 3.14159, 1.537e-38, 1.673e25, 6.02214179e23, -6.02214179e23, INFINITY.Float32(), NEGATIVE_INFINITY.Float32(), NAN.Float32()}

	thetype := Type(FLOAT)
	thelen := len(floatValues)
	p.WriteListBegin(thetype, thelen)
	for _, v := range floatValues {
		p.WriteFloat(v)
	}
	p.WriteListEnd()
	p.Flush()
	thetype2, thelen2, err := p.ReadListBegin()
	if err != nil {
		t.Errorf("%s: %T %T %q Error reading list: %v", "ReadWriteFloat", p, trans, err, floatValues)
	}
	if thetype != thetype2 {
		t.Errorf("%s: %T %T type %s != type %s", "ReadWriteFloat", p, trans, thetype, thetype2)
	}
	if thelen != thelen2 {
		t.Errorf("%s: %T %T len %d != len %d", "ReadWriteFloat", p, trans, thelen, thelen2)
	}
	for k, v := range floatValues {
		value, err := p.ReadFloat()
		if err != nil {
			t.Errorf("%s: %T %T %q Error reading double at index %d: %f", "ReadWriteFloat", p, trans, err, k, v)
		}
		if math.IsNaN(float64(v)) {
			if !math.IsNaN(float64(value)) {
				t.Errorf("%s: %T %T math.IsNaN(%f) != math.IsNaN(%f)", "ReadWriteFloat", p, trans, v, value)
			}
		} else if v != value {
			t.Errorf("%s: %T %T %f != %f", "ReadWriteFloat", p, trans, v, value)
		}
	}
	err = p.ReadListEnd()
	if err != nil {
		t.Errorf("%s: %T %T Unable to read list end: %q", "ReadWriteFloat", p, trans, err)
	}
}

func ReadWriteString(t testing.TB, p Protocol, trans Transport) {
	thetype := Type(STRING)
	thelen := len(stringValues)
	p.WriteListBegin(thetype, thelen)
	for _, v := range stringValues {
		p.WriteString(v)
	}
	p.WriteListEnd()
	p.Flush()
	thetype2, thelen2, err := p.ReadListBegin()
	if err != nil {
		t.Errorf("%s: %T %T %q Error reading list: %q", "ReadWriteString", p, trans, err, stringValues)
	}
	_, ok := p.(*SimpleJSONProtocol)
	if !ok {
		if thetype != thetype2 {
			t.Errorf("%s: %T %T type %s != type %s", "ReadWriteString", p, trans, thetype, thetype2)
		}
		if thelen != thelen2 {
			t.Errorf("%s: %T %T len %d != len %d", "ReadWriteString", p, trans, thelen, thelen2)
		}
	}
	for k, v := range stringValues {
		value, err := p.ReadString()
		if err != nil {
			t.Errorf("%s: %T %T %q Error reading string at index %d: %q", "ReadWriteString", p, trans, err, k, v)
		}
		if v != value {
			t.Errorf("%s: %T %T %s != %s", "ReadWriteString", p, trans, v, value)
		}
	}
	if err != nil {
		t.Errorf("%s: %T %T Unable to read list end: %q", "ReadWriteString", p, trans, err)
	}
}

func ReadWriteBinary(t testing.TB, p Protocol, trans Transport) {
	v := protocol_bdata
	p.WriteBinary(v)
	p.Flush()
	value, err := p.ReadBinary()
	if err != nil {
		t.Errorf("%s: %T %T Unable to read binary: %s", "ReadWriteBinary", p, trans, err.Error())
	}
	if len(v) != len(value) {
		t.Errorf("%s: %T %T len(v) != len(value)... %d != %d", "ReadWriteBinary", p, trans, len(v), len(value))
	} else {
		for i := 0; i < len(v); i++ {
			if v[i] != value[i] {
				t.Errorf("%s: %T %T %s != %s", "ReadWriteBinary", p, trans, v, value)
			}
		}
	}
}
