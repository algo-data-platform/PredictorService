from enum import Enum
from folly.iobuf import IOBuf


__all__ = ['Struct', 'BadEnum', 'NOTSET', 'Union']

class NOTSETTYPE(Enum):
    token = 0

NOTSET = NOTSETTYPE.token


cdef class Struct:
    """
    Base class for all thrift structs
    """
    cdef IOBuf _serialize(self, proto):
        return IOBuf(b'')

    cdef uint32_t _deserialize(self, const cIOBuf* buf, proto) except? 0:
        return 0


cdef class Union(Struct):
    """
    Base class for all thrift Unions
    """
    pass


cdef class BadEnum:
    """
    This represents a BadEnum value from thrift.
    So an out of date thrift definition or a default value that is not
    in the enum
    """

    def __init__(self, the_enum, value):
        self._enum = the_enum
        self.value = value
        self.name = '#INVALID#'

    def __repr__(self):
        return f'<{self.enum.__name__}.{self.name}: {self.value}>'

    def __int__(self):
        return self.value

    @property
    def enum(self):
        return self._enum


cdef translate_cpp_enum_to_python(object EnumClass, int value):
    try:
        return EnumClass(value)
    except ValueError:
        return BadEnum(EnumClass, value)
