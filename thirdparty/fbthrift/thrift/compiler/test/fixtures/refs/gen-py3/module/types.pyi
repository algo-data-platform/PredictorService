#
# Autogenerated by Thrift
#
# DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
#  @generated
#

from folly.iobuf import IOBuf as __IOBuf
import thrift.py3.types
import thrift.py3.exceptions
from thrift.py3.types import NOTSET, NOTSETTYPE
from thrift.py3.serializer import Protocol
import typing as _typing

import sys
import itertools
import enum as __enum


class TypedEnum(__enum.Enum, _typing.SupportsInt):
    VAL1: TypedEnum = ...
    VAL2: TypedEnum = ...
    value: int
    def __int__(self) -> int: ...


class MyUnion(thrift.py3.types.Union, _typing.Hashable):
    def __init__(
        self, *,
        anInteger: _typing.Optional[int]=None,
        aString: _typing.Optional[str]=None
    ) -> None: ...

    def __bool__(self) -> bool: ...
    def __hash__(self) -> int: ...
    def __repr__(self) -> str: ...
    def __lt__(self, other: 'MyUnion') -> bool: ...

    @property
    def anInteger(self) -> int: ...
    @property
    def aString(self) -> str: ...
    class Type(__enum.Enum):
        EMPTY: MyUnion.Type = ...
        anInteger: MyUnion.Type = ...
        aString: MyUnion.Type = ...
        value: int

    @staticmethod
    def fromValue(value: _typing.Union[int, str]) -> MyUnion: ...
    @property
    def value(self) -> _typing.Union[int, str]: ...
    @property
    def type(self) -> "MyUnion.Type": ...
    def get_type(self) -> "MyUnion.Type": ...


class MyField(thrift.py3.types.Struct, _typing.Hashable, _typing.Iterable[_typing.Tuple[str, _typing.Any]]):
    def __init__(
        self, *,
        opt_value: _typing.Optional[int]=None,
        value: _typing.Optional[int]=None,
        req_value: int
    ) -> None: ...

    def __call__(
        self, *,
        opt_value: _typing.Union[int, NOTSETTYPE, None]=NOTSET,
        value: _typing.Union[int, NOTSETTYPE, None]=NOTSET,
        req_value: _typing.Union[int, NOTSETTYPE]=NOTSET
    ) -> MyField: ...

    def __reduce__(self) -> _typing.Tuple[_typing.Callable, _typing.Tuple[_typing.Type['MyField'], bytes]]: ...
    def __iter__(self) -> _typing.Iterator[_typing.Tuple[str, _typing.Any]]: ...
    def __bool__(self) -> bool: ...
    def __hash__(self) -> int: ...
    def __repr__(self) -> str: ...
    def __lt__(self, other: 'MyField') -> bool: ...

    @property
    def opt_value(self) -> _typing.Optional[int]: ...
    @property
    def value(self) -> int: ...
    @property
    def req_value(self) -> int: ...


class MyStruct(thrift.py3.types.Struct, _typing.Hashable, _typing.Iterable[_typing.Tuple[str, _typing.Any]]):
    def __init__(
        self, *,
        opt_ref: _typing.Optional['MyField']=None,
        ref: _typing.Optional['MyField']=None,
        req_ref: 'MyField'
    ) -> None: ...

    def __call__(
        self, *,
        opt_ref: _typing.Union['MyField', NOTSETTYPE, None]=NOTSET,
        ref: _typing.Union['MyField', NOTSETTYPE, None]=NOTSET,
        req_ref: _typing.Union['MyField', NOTSETTYPE]=NOTSET
    ) -> MyStruct: ...

    def __reduce__(self) -> _typing.Tuple[_typing.Callable, _typing.Tuple[_typing.Type['MyStruct'], bytes]]: ...
    def __iter__(self) -> _typing.Iterator[_typing.Tuple[str, _typing.Any]]: ...
    def __bool__(self) -> bool: ...
    def __hash__(self) -> int: ...
    def __repr__(self) -> str: ...
    def __lt__(self, other: 'MyStruct') -> bool: ...

    @property
    def opt_ref(self) -> _typing.Optional['MyField']: ...
    @property
    def ref(self) -> _typing.Optional['MyField']: ...
    @property
    def req_ref(self) -> 'MyField': ...


class StructWithUnion(thrift.py3.types.Struct, _typing.Hashable, _typing.Iterable[_typing.Tuple[str, _typing.Any]]):
    def __init__(
        self, *,
        u: _typing.Optional['MyUnion']=None,
        aDouble: _typing.Optional[float]=None,
        f: _typing.Optional['MyField']=None
    ) -> None: ...

    def __call__(
        self, *,
        u: _typing.Union['MyUnion', NOTSETTYPE, None]=NOTSET,
        aDouble: _typing.Union[float, NOTSETTYPE, None]=NOTSET,
        f: _typing.Union['MyField', NOTSETTYPE, None]=NOTSET
    ) -> StructWithUnion: ...

    def __reduce__(self) -> _typing.Tuple[_typing.Callable, _typing.Tuple[_typing.Type['StructWithUnion'], bytes]]: ...
    def __iter__(self) -> _typing.Iterator[_typing.Tuple[str, _typing.Any]]: ...
    def __bool__(self) -> bool: ...
    def __hash__(self) -> int: ...
    def __repr__(self) -> str: ...
    def __lt__(self, other: 'StructWithUnion') -> bool: ...

    @property
    def u(self) -> _typing.Optional['MyUnion']: ...
    @property
    def aDouble(self) -> float: ...
    @property
    def f(self) -> 'MyField': ...


class RecursiveStruct(thrift.py3.types.Struct, _typing.Hashable, _typing.Iterable[_typing.Tuple[str, _typing.Any]]):
    def __init__(
        self, *,
        mes: _typing.Optional[_typing.Sequence['RecursiveStruct']]=None
    ) -> None: ...

    def __call__(
        self, *,
        mes: _typing.Union[_typing.Sequence['RecursiveStruct'], NOTSETTYPE, None]=NOTSET
    ) -> RecursiveStruct: ...

    def __reduce__(self) -> _typing.Tuple[_typing.Callable, _typing.Tuple[_typing.Type['RecursiveStruct'], bytes]]: ...
    def __iter__(self) -> _typing.Iterator[_typing.Tuple[str, _typing.Any]]: ...
    def __bool__(self) -> bool: ...
    def __hash__(self) -> int: ...
    def __repr__(self) -> str: ...
    def __lt__(self, other: 'RecursiveStruct') -> bool: ...

    @property
    def mes(self) -> _typing.Optional[_typing.Sequence['RecursiveStruct']]: ...


class StructWithContainers(thrift.py3.types.Struct, _typing.Hashable, _typing.Iterable[_typing.Tuple[str, _typing.Any]]):
    def __init__(
        self, *,
        list_ref: _typing.Optional[_typing.Sequence[int]]=None,
        set_ref: _typing.Optional[_typing.AbstractSet[int]]=None,
        map_ref: _typing.Optional[_typing.Mapping[int, int]]=None,
        list_ref_unique: _typing.Optional[_typing.Sequence[int]]=None,
        set_ref_shared: _typing.Optional[_typing.AbstractSet[int]]=None,
        list_ref_shared_const: _typing.Optional[_typing.Sequence[int]]=None
    ) -> None: ...

    def __call__(
        self, *,
        list_ref: _typing.Union[_typing.Sequence[int], NOTSETTYPE, None]=NOTSET,
        set_ref: _typing.Union[_typing.AbstractSet[int], NOTSETTYPE, None]=NOTSET,
        map_ref: _typing.Union[_typing.Mapping[int, int], NOTSETTYPE, None]=NOTSET,
        list_ref_unique: _typing.Union[_typing.Sequence[int], NOTSETTYPE, None]=NOTSET,
        set_ref_shared: _typing.Union[_typing.AbstractSet[int], NOTSETTYPE, None]=NOTSET,
        list_ref_shared_const: _typing.Union[_typing.Sequence[int], NOTSETTYPE, None]=NOTSET
    ) -> StructWithContainers: ...

    def __reduce__(self) -> _typing.Tuple[_typing.Callable, _typing.Tuple[_typing.Type['StructWithContainers'], bytes]]: ...
    def __iter__(self) -> _typing.Iterator[_typing.Tuple[str, _typing.Any]]: ...
    def __bool__(self) -> bool: ...
    def __hash__(self) -> int: ...
    def __repr__(self) -> str: ...
    def __lt__(self, other: 'StructWithContainers') -> bool: ...

    @property
    def list_ref(self) -> _typing.Optional[_typing.Sequence[int]]: ...
    @property
    def set_ref(self) -> _typing.Optional[_typing.AbstractSet[int]]: ...
    @property
    def map_ref(self) -> _typing.Optional[_typing.Mapping[int, int]]: ...
    @property
    def list_ref_unique(self) -> _typing.Optional[_typing.Sequence[int]]: ...
    @property
    def set_ref_shared(self) -> _typing.Optional[_typing.AbstractSet[int]]: ...
    @property
    def list_ref_shared_const(self) -> _typing.Optional[_typing.Sequence[int]]: ...


class StructWithSharedConst(thrift.py3.types.Struct, _typing.Hashable, _typing.Iterable[_typing.Tuple[str, _typing.Any]]):
    def __init__(
        self, *,
        opt_shared_const: _typing.Optional['MyField']=None,
        shared_const: _typing.Optional['MyField']=None,
        req_shared_const: 'MyField'
    ) -> None: ...

    def __call__(
        self, *,
        opt_shared_const: _typing.Union['MyField', NOTSETTYPE, None]=NOTSET,
        shared_const: _typing.Union['MyField', NOTSETTYPE, None]=NOTSET,
        req_shared_const: _typing.Union['MyField', NOTSETTYPE]=NOTSET
    ) -> StructWithSharedConst: ...

    def __reduce__(self) -> _typing.Tuple[_typing.Callable, _typing.Tuple[_typing.Type['StructWithSharedConst'], bytes]]: ...
    def __iter__(self) -> _typing.Iterator[_typing.Tuple[str, _typing.Any]]: ...
    def __bool__(self) -> bool: ...
    def __hash__(self) -> int: ...
    def __repr__(self) -> str: ...
    def __lt__(self, other: 'StructWithSharedConst') -> bool: ...

    @property
    def opt_shared_const(self) -> _typing.Optional['MyField']: ...
    @property
    def shared_const(self) -> _typing.Optional['MyField']: ...
    @property
    def req_shared_const(self) -> 'MyField': ...


class Empty(thrift.py3.types.Struct, _typing.Hashable, _typing.Iterable[_typing.Tuple[str, _typing.Any]]):
    def __init__(
        self, 
    ) -> None: ...

    def __call__(
        self, 
    ) -> Empty: ...

    def __reduce__(self) -> _typing.Tuple[_typing.Callable, _typing.Tuple[_typing.Type['Empty'], bytes]]: ...
    def __iter__(self) -> _typing.Iterator[_typing.Tuple[str, _typing.Any]]: ...
    def __bool__(self) -> bool: ...
    def __hash__(self) -> int: ...
    def __repr__(self) -> str: ...
    def __lt__(self, other: 'Empty') -> bool: ...



class StructWithRef(thrift.py3.types.Struct, _typing.Hashable, _typing.Iterable[_typing.Tuple[str, _typing.Any]]):
    def __init__(
        self, *,
        def_field: _typing.Optional['Empty']=None,
        opt_field: _typing.Optional['Empty']=None,
        req_field: 'Empty'
    ) -> None: ...

    def __call__(
        self, *,
        def_field: _typing.Union['Empty', NOTSETTYPE, None]=NOTSET,
        opt_field: _typing.Union['Empty', NOTSETTYPE, None]=NOTSET,
        req_field: _typing.Union['Empty', NOTSETTYPE]=NOTSET
    ) -> StructWithRef: ...

    def __reduce__(self) -> _typing.Tuple[_typing.Callable, _typing.Tuple[_typing.Type['StructWithRef'], bytes]]: ...
    def __iter__(self) -> _typing.Iterator[_typing.Tuple[str, _typing.Any]]: ...
    def __bool__(self) -> bool: ...
    def __hash__(self) -> int: ...
    def __repr__(self) -> str: ...
    def __lt__(self, other: 'StructWithRef') -> bool: ...

    @property
    def def_field(self) -> _typing.Optional['Empty']: ...
    @property
    def opt_field(self) -> _typing.Optional['Empty']: ...
    @property
    def req_field(self) -> 'Empty': ...


class StructWithRefTypeUnique(thrift.py3.types.Struct, _typing.Hashable, _typing.Iterable[_typing.Tuple[str, _typing.Any]]):
    def __init__(
        self, *,
        def_field: _typing.Optional['Empty']=None,
        opt_field: _typing.Optional['Empty']=None,
        req_field: 'Empty'
    ) -> None: ...

    def __call__(
        self, *,
        def_field: _typing.Union['Empty', NOTSETTYPE, None]=NOTSET,
        opt_field: _typing.Union['Empty', NOTSETTYPE, None]=NOTSET,
        req_field: _typing.Union['Empty', NOTSETTYPE]=NOTSET
    ) -> StructWithRefTypeUnique: ...

    def __reduce__(self) -> _typing.Tuple[_typing.Callable, _typing.Tuple[_typing.Type['StructWithRefTypeUnique'], bytes]]: ...
    def __iter__(self) -> _typing.Iterator[_typing.Tuple[str, _typing.Any]]: ...
    def __bool__(self) -> bool: ...
    def __hash__(self) -> int: ...
    def __repr__(self) -> str: ...
    def __lt__(self, other: 'StructWithRefTypeUnique') -> bool: ...

    @property
    def def_field(self) -> _typing.Optional['Empty']: ...
    @property
    def opt_field(self) -> _typing.Optional['Empty']: ...
    @property
    def req_field(self) -> 'Empty': ...


class StructWithRefTypeShared(thrift.py3.types.Struct, _typing.Hashable, _typing.Iterable[_typing.Tuple[str, _typing.Any]]):
    def __init__(
        self, *,
        def_field: _typing.Optional['Empty']=None,
        opt_field: _typing.Optional['Empty']=None,
        req_field: 'Empty'
    ) -> None: ...

    def __call__(
        self, *,
        def_field: _typing.Union['Empty', NOTSETTYPE, None]=NOTSET,
        opt_field: _typing.Union['Empty', NOTSETTYPE, None]=NOTSET,
        req_field: _typing.Union['Empty', NOTSETTYPE]=NOTSET
    ) -> StructWithRefTypeShared: ...

    def __reduce__(self) -> _typing.Tuple[_typing.Callable, _typing.Tuple[_typing.Type['StructWithRefTypeShared'], bytes]]: ...
    def __iter__(self) -> _typing.Iterator[_typing.Tuple[str, _typing.Any]]: ...
    def __bool__(self) -> bool: ...
    def __hash__(self) -> int: ...
    def __repr__(self) -> str: ...
    def __lt__(self, other: 'StructWithRefTypeShared') -> bool: ...

    @property
    def def_field(self) -> _typing.Optional['Empty']: ...
    @property
    def opt_field(self) -> _typing.Optional['Empty']: ...
    @property
    def req_field(self) -> 'Empty': ...


class StructWithRefTypeSharedConst(thrift.py3.types.Struct, _typing.Hashable, _typing.Iterable[_typing.Tuple[str, _typing.Any]]):
    def __init__(
        self, *,
        def_field: _typing.Optional['Empty']=None,
        opt_field: _typing.Optional['Empty']=None,
        req_field: 'Empty'
    ) -> None: ...

    def __call__(
        self, *,
        def_field: _typing.Union['Empty', NOTSETTYPE, None]=NOTSET,
        opt_field: _typing.Union['Empty', NOTSETTYPE, None]=NOTSET,
        req_field: _typing.Union['Empty', NOTSETTYPE]=NOTSET
    ) -> StructWithRefTypeSharedConst: ...

    def __reduce__(self) -> _typing.Tuple[_typing.Callable, _typing.Tuple[_typing.Type['StructWithRefTypeSharedConst'], bytes]]: ...
    def __iter__(self) -> _typing.Iterator[_typing.Tuple[str, _typing.Any]]: ...
    def __bool__(self) -> bool: ...
    def __hash__(self) -> int: ...
    def __repr__(self) -> str: ...
    def __lt__(self, other: 'StructWithRefTypeSharedConst') -> bool: ...

    @property
    def def_field(self) -> _typing.Optional['Empty']: ...
    @property
    def opt_field(self) -> _typing.Optional['Empty']: ...
    @property
    def req_field(self) -> 'Empty': ...


_List__RecursiveStructT = _typing.TypeVar('_List__RecursiveStructT', bound=_typing.Sequence['RecursiveStruct'])


class List__RecursiveStruct(_typing.Sequence['RecursiveStruct'], _typing.Hashable):
    def __init__(self, items: _typing.Sequence['RecursiveStruct']=None) -> None: ...
    def __repr__(self) -> str: ...
    def __len__(self) -> int: ...
    def __hash__(self) -> int: ...
    def __contains__(self, x: object) -> bool: ...
    @_typing.overload
    def __getitem__(self, i: int) -> 'RecursiveStruct': ...
    @_typing.overload
    def __getitem__(self, s: slice) -> _typing.Sequence['RecursiveStruct']: ...
    def count(self, item: _typing.Any) -> int: ...
    def index(self, item: _typing.Any, start: int = ..., stop: int = ...) -> int: ...
    def __add__(self, other: _typing.Sequence['RecursiveStruct']) -> 'List__RecursiveStruct': ...
    def __radd__(self, other: _List__RecursiveStructT) -> _List__RecursiveStructT: ...
    def __reversed__(self) -> _typing.Iterator['RecursiveStruct']: ...
    def __iter__(self) -> _typing.Iterator['RecursiveStruct']: ...


_List__i32T = _typing.TypeVar('_List__i32T', bound=_typing.Sequence[int])


class List__i32(_typing.Sequence[int], _typing.Hashable):
    def __init__(self, items: _typing.Sequence[int]=None) -> None: ...
    def __repr__(self) -> str: ...
    def __len__(self) -> int: ...
    def __hash__(self) -> int: ...
    def __contains__(self, x: object) -> bool: ...
    @_typing.overload
    def __getitem__(self, i: int) -> int: ...
    @_typing.overload
    def __getitem__(self, s: slice) -> _typing.Sequence[int]: ...
    def count(self, item: _typing.Any) -> int: ...
    def index(self, item: _typing.Any, start: int = ..., stop: int = ...) -> int: ...
    def __add__(self, other: _typing.Sequence[int]) -> 'List__i32': ...
    def __radd__(self, other: _List__i32T) -> _List__i32T: ...
    def __reversed__(self) -> _typing.Iterator[int]: ...
    def __iter__(self) -> _typing.Iterator[int]: ...


class Set__i32(_typing.AbstractSet[int], _typing.Hashable):
    def __init__(self, items: _typing.AbstractSet[int]=None) -> None: ...
    def __repr__(self) -> str: ...
    def __len__(self) -> int: ...
    def __hash__(self) -> int: ...
    def __contains__(self, x: object) -> bool: ...
    def isdisjoint(self, other: _typing.AbstractSet[int]) -> bool: ...
    def union(self, other: _typing.AbstractSet[int]) -> 'Set__i32': ...
    def intersection(self, other: _typing.AbstractSet[int]) -> 'Set__i32': ...
    def difference(self, other: _typing.AbstractSet[int]) -> 'Set__i32': ...
    def symmetric_difference(self, other: _typing.AbstractSet[int]) -> 'Set__i32': ...
    def issubset(self, other: _typing.AbstractSet[int]) -> bool: ...
    def issuperset(self, other: _typing.AbstractSet[int]) -> bool: ...
    def __iter__(self) -> _typing.Iterator[int]: ...


class Map__i32_i32(_typing.Mapping[int, int], _typing.Hashable):
    def __init__(self, items: _typing.Mapping[int, int]=None) -> None: ...
    def __repr__(self) -> str: ...
    def __len__(self) -> int: ...
    def __hash__(self) -> int: ...
    def __contains__(self, x: object) -> bool: ...
    def __getitem__(self, key: int) -> int: ...
    def __iter__(self) -> _typing.Iterator[int]: ...


kStructWithRef: StructWithRef = ...
kStructWithRefTypeUnique: StructWithRefTypeUnique = ...
kStructWithRefTypeShared: StructWithRefTypeShared = ...
kStructWithRefTypeSharedConst: StructWithRefTypeSharedConst = ...
