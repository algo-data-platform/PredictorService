/**
 * Autogenerated by Thrift
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */

package test.fixtures.basic;

import com.facebook.swift.codec.*;
import com.facebook.swift.codec.ThriftField.Requiredness;
import com.facebook.swift.service.*;
import com.google.common.util.concurrent.ListenableFuture;
import java.io.*;
import java.util.*;

@ThriftService("MyServicePrioParent")
public interface MyServicePrioParent
{
    @ThriftService("MyServicePrioParent")
    public interface Async
    {
        @ThriftMethod(value = "ping")
        ListenableFuture<Void> ping(
        );

        @ThriftMethod(value = "pong")
        ListenableFuture<Void> pong(
        );
    }
    @ThriftMethod(value = "ping")
    void ping(
    );


    @ThriftMethod(value = "pong")
    void pong(
    );

}
