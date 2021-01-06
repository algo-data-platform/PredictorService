#!/usr/bin/env python3
import asyncio
from pathlib import Path
import socket
import tempfile
import unittest

from testing.services import TestingServiceInterface
from testing.types import easy, Color
from testing.clients import TestingService
from thrift.py3 import ThriftServer, get_client, TransportError, RequestContext, RpcOptions
from typing import Sequence, Optional
import thrift.py3.server


class Handler(TestingServiceInterface):

    @TestingServiceInterface.pass_context_invert
    async def invert(self, ctx: RequestContext, value: bool) -> bool:
        if 'from client' in ctx.read_headers:
            ctx.set_header('from server', 'with love')
        return not value

    async def getName(self) -> str:
        return "Testing"

    async def shutdown(self) -> None:
        pass

    async def complex_action(
        self, first: str, second: str, third: int, fourth: str
    ) -> int:
        return third

    async def takes_a_list(self, ints: Sequence[int]) -> None:
        pass

    async def take_it_easy(self, how: int, what: easy) -> None:
        pass

    async def pick_a_color(self, color: Color) -> None:
        pass

    async def int_sizes(self, one: int, two: int, three: int, four: int) -> None:
        pass


class TestServer:
    server: ThriftServer
    serve_task: asyncio.Task

    def __init__(
        self, ip: Optional[str] = None, path: Optional['thrift.py3.server.Path'] = None
    ) -> None:
        self.server = ThriftServer(Handler(), ip=ip, path=path)

    async def __aenter__(self) -> thrift.py3.server.SocketAddress:
        self.serve_task = asyncio.get_event_loop().create_task(self.server.serve())
        return await self.server.get_address()

    async def __aexit__(self, *exc_info) -> None:
        self.server.stop()
        await self.serve_task


class ClientServerTests(unittest.TestCase):
    """
    These are tests where a client and server talk to each other
    """
    def test_rpc_headers(self) -> None:
        loop = asyncio.get_event_loop()

        async def inner_test() -> None:
            async with TestServer(ip='::1') as sa:
                assert sa.ip and sa.port
                async with get_client(
                    TestingService, host=sa.ip, port=sa.port
                ) as client:
                    options = RpcOptions()
                    options.set_header('from client', 'with love')
                    self.assertFalse(await client.invert(True, rpc_options=options))
                    self.assertIn('from server', options.read_headers)

        loop.run_until_complete(inner_test())

    def test_client_resolve(self) -> None:
        loop = asyncio.get_event_loop()
        hostname = socket.gethostname()

        async def inner_test() -> None:
            async with TestServer() as sa:
                assert sa.port
                async with get_client(
                    TestingService, host=hostname, port=sa.port
                ) as client:
                    self.assertTrue(await client.invert(False))
                    self.assertFalse(await client.invert(True))

        loop.run_until_complete(inner_test())

    def test_server_localhost(self) -> None:
        loop = asyncio.get_event_loop()

        async def inner_test() -> None:
            async with TestServer(ip='::1') as sa:
                assert sa.ip and sa.port
                async with get_client(
                    TestingService, host=sa.ip, port=sa.port
                ) as client:
                    self.assertTrue(await client.invert(False))
                    self.assertFalse(await client.invert(True))

        loop.run_until_complete(inner_test())

    def test_unix_socket(self) -> None:
        loop = asyncio.get_event_loop()

        async def inner_test(dir: Path) -> None:
            async with TestServer(path=dir / 'tserver.sock') as sa:
                assert sa.path
                for r in range(2):
                    try:
                        async with get_client(TestingService, path=sa.path) as client:
                            self.assertTrue(await client.invert(False))
                            self.assertFalse(await client.invert(True))
                        break

                    except TransportError:
                        if r == 1:
                            raise

        with tempfile.TemporaryDirectory() as tdir:
            loop.run_until_complete(inner_test(Path(tdir)))

    def test_no_client_aexit(self) -> None:
        loop = asyncio.get_event_loop()

        async def inner_test() -> None:
            async with TestServer() as sa:
                assert sa.port and sa.ip
                client = get_client(TestingService, host=sa.ip, port=sa.port)
                await client.__aenter__()
                self.assertTrue(await client.invert(False))
                self.assertFalse(await client.invert(True))

        # If we do not abort here then good

        loop.run_until_complete(inner_test())

    def test_client_aexit_cancel(self) -> None:
        """
        This actually handles the case if __aexit__ is not awaited
        """
        loop = asyncio.get_event_loop()

        async def inner_test() -> None:
            async with TestServer() as sa:
                assert sa.port and sa.ip
                client = get_client(TestingService, host=sa.ip, port=sa.port)
                await client.__aenter__()
                self.assertTrue(await client.invert(False))
                self.assertFalse(await client.invert(True))
                fut = client.__aexit__(None, None, None)
                fut.cancel()  # type: ignore
                del client  # If we do not abort here then good

        loop.run_until_complete(inner_test())

    def test_no_client_no_aenter(self) -> None:
        """
        This covers if aenter was canceled since those two are the same really
        """
        loop = asyncio.get_event_loop()

        async def inner_test() -> None:
            async with TestServer() as sa:
                assert sa.port and sa.ip
                get_client(TestingService, host=sa.ip, port=sa.port)

        # If we do not abort here then good

        loop.run_until_complete(inner_test())
