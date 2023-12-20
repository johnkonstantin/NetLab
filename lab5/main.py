import asyncio
import sys
import logging
from colorama import Fore
import socket

USERNAMEPASSWD_METHOD = 0x02
NOACCEPTABLE_METHOD = 0xff
SOCKS_VERSION = 0x05
AUTHNEG_VERSION = 0x01
AUTH_SUCCESS = 0x00
AUTH_FAILURE = 0xff
CONNECT = 0x01
ATYPE_IPV4 = 0x01
ATYPE_DN = 0x03

UNAME = 'giga'
PASSWD = 'niga'

BUFFSIZE = 4096


async def proxy_uname_auth(reader, writer) -> bool:
    addr = writer.transport.get_extra_info("peername")
    buf = await reader.readexactly(1)
    ver = int(buf[0])
    logging.info(str(addr) + ": version is " + str(ver))
    if ver != SOCKS_VERSION:
        logging.info(str(addr) + ": refusing connection")
        return False

    buf = await reader.readexactly(1)
    nmethods = int(buf[0])
    logging.info(str(addr) + ": nmethods is " + str(nmethods))

    buf = await reader.readexactly(nmethods)
    logging.info(str(addr) + ": methods are " + buf.hex())
    if USERNAMEPASSWD_METHOD not in buf:
        logging.info(str(addr) + ": methods do not include username/password auth")
        writer.write(bytes([SOCKS_VERSION, NOACCEPTABLE_METHOD]))
        await writer.drain()
        return False
    writer.write(bytes([SOCKS_VERSION, USERNAMEPASSWD_METHOD]))
    await writer.drain()

    buf = await reader.readexactly(1)
    neg = int(buf[0])
    if (neg != AUTHNEG_VERSION):
        logging.info(str(addr) + ": not supported username/password auth version")
        writer.write(bytes([neg, AUTH_FAILURE]))
        await writer.drain()
        return False
    buf = await reader.readexactly(1)
    ulen = int(buf[0])
    buf = await reader.readexactly(ulen)
    uname = buf.decode()
    logging.info(str(addr) + ": uname: " + uname)
    buf = await reader.readexactly(1)
    plen = int(buf[0])
    buf = await reader.readexactly(ulen)
    passwd = buf.decode()
    logging.info(str(addr) + ": passwd: " + passwd)

    if uname != UNAME or passwd != PASSWD:
        logging.info(str(addr) + ": not valid uname or passwd")
        writer.write(bytes([neg, AUTH_FAILURE]))
        await writer.drain()
        return False
    logging.info(str(addr) + ": auth is successful")
    writer.write(bytes([neg, AUTH_SUCCESS]))
    await writer.drain()
    return True


async def proxy_get_user_request(reader: asyncio.StreamReader, writer: asyncio.StreamWriter) -> tuple:
    addr = writer.get_extra_info('peername')
    buf = await reader.readexactly(1)
    ver = int(buf[0])
    logging.info(str(addr) + ": request version is " + str(ver))
    if ver != SOCKS_VERSION:
        logging.info(str(addr) + ": refusing connection")
        return None

    buf = await reader.readexactly(1)
    cmd = int(buf[0])
    logging.info(str(addr) + ": command is " + str(cmd))
    if cmd != CONNECT:
        logging.info(str(addr) + ": unsupported command")
        writer.write(b'\x05\x07\x00\x01\x00\x00\x00\x00\x00\x00')
        await writer.drain()
        return None

    buf = await reader.readexactly(1)  # RSV
    buf = await reader.readexactly(1)
    atyp = int(buf[0])
    if atyp == ATYPE_IPV4:
        buf = await reader.readexactly(4)
        host = socket.inet_ntoa(buf)
    elif atyp == ATYPE_DN:
        buf = await reader.readexactly(1)
        nlen = int(buf[0])
        buf = await reader.readexactly(nlen)
        host = socket.gethostbyname(buf)
    else:
        writer.write(b'\x05\x08\x00\x01\x00\x00\x00\x00\x00\x00')
        await writer.drain()
        return None
    logging.info(str(addr) + ": host: " + host)
    buf = await reader.readexactly(2)
    port = int.from_bytes(buf, 'big')
    logging.info(str(addr) + ": host: " + str(port))

    return host, port


async def reader2writer(reader: asyncio.StreamReader, writer: asyncio.StreamWriter):
    while not reader.at_eof():
        buf = await reader.read(BUFFSIZE)
        writer.write(buf)
        await writer.drain()
    writer.write_eof()
    await writer.drain()


async def proxy_client_connected(reader: asyncio.StreamReader, writer: asyncio.StreamWriter):
    try:
        addr = writer.get_extra_info('peername')
        logging.info(str(addr) + ": connected")
        res = await proxy_uname_auth(reader, writer)
        if not res:
            writer.write_eof()
            await writer.drain()
            writer.close()
            await writer.wait_closed()
            return

        host, port = await proxy_get_user_request(reader, writer)
    except asyncio.IncompleteReadError:
        logging.info(str(addr) + ": client closed connection")
        writer.close()
        await writer.wait_closed()
        return

    try:
        conn_r, conn_w = await asyncio.open_connection(host, port)
    except ConnectionRefusedError:
        logging.info(str(addr) + ": connection refused to " + host + ":" + str(port))
        writer.write(b'\x05\x05\x00\x01\x00\x00\x00\x00\x00\x00')
        writer.write_eof()
        await writer.drain()
        writer.close()
        await writer.wait_closed()
        return
    except TimeoutError:
        logging.info(str(addr) + ": unreachable host: " + host + ":" + str(port))
        writer.write(b'\x05\x04\x00\x01\x00\x00\x00\x00\x00\x00')
        writer.write_eof()
        await writer.drain()
        writer.close()
        await writer.wait_closed()
        return
    except:
        logging.info(str(addr) + ": error while connecting to " + host + ":" + str(port))
        writer.write(b'\x05\x01\x00\x01\x00\x00\x00\x00\x00\x00')
        writer.write_eof()
        await writer.drain()
        writer.close()
        await writer.wait_closed()
        return

    logging.info(str(addr) + ": connected successfully to " + host + ":" + str(port))
    rhost, rport = conn_w.get_extra_info('sockname')
    writer.write(b'\x05\x00\x00\x01' + socket.inet_aton(rhost) + rport.to_bytes(2, 'big'))
    try:
        await asyncio.gather(
            reader2writer(reader, conn_w),
            reader2writer(conn_r, writer)
        )
    except:
        logging.info(str(addr) + ": error while forwarding to " + host + ":" + str(port))
    finally:
        logging.info(str(addr) + ": connection closed to " + host + ":" + str(port))
        writer.close()
        conn_w.close()


async def main(host, port):
    logging.info(f"Started serving proxy at {Fore.GREEN}" + host + ":" + str(port) + f"{Fore.RESET}")
    serv = await asyncio.start_server(proxy_client_connected, host, port)
    while True:
        print(len(asyncio.all_tasks()))
        await asyncio.sleep(5)
    # await serv.serve_forever()


if __name__ == "__main__":
    logging.getLogger().name = 'aiosocks'
    logging.getLogger().setLevel(logging.INFO)
    if len(sys.argv) < 2:
        print(f"usage: {Fore.CYAN}aiosocks{Fore.RESET} {Fore.GREEN}<port>{Fore.RESET}")
        logging.exception(f"{Fore.RED}bad number of argumetns{Fore.RESET}")
    else:
        asyncio.run(main('', int(sys.argv[1])))
