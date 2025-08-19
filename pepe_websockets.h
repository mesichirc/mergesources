#ifndef PEPE_WEBSOCKETS_H
#define PEPE_WEBSOCKETS_H

bool
Pepe_UpgradeToWebsocketProtocol(Pepe_HttpRequest *request, Pepe_HttpResponse *response)
{
  Pepe_String wsKeyHeader;
  Pepe_String wsResponseKey;

  if (!Pepe_HttpHeadersHasValueCString(&request->headers, "Upgrade", "websocket")) {
    return false;
  }
  if (!Pepe_HttpHeadersHasValueCString(&request->headers, "Connection", "Upgrade")) {
    return false;
  }

  wsKeyHeader = Pepe_HttpHeadersGet(&request->headers, Pepe_StringFromCString("Sec-Websocket-Key"));
  if (wsKeyHeader.length == 0) {
    return false;
  }

  wsResponseKey = Pepe_WebsocketSecWebsocketAcceptHeader(response->arena, wsKeyHeader);

  Pepe_HttpResponseWriteCString(response, "HTTP/1.1 101 Switching Protocols\r\n"); 
  Pepe_HttpResponseWriteCString(response, "Upgrade: websocket\r\n");
  Pepe_HttpResponseWriteCString(response, "Connection: Upgrade\r\n");
  Pepe_HttpResponseWriteCString(response, "Sec-WebSocket-Accept: ");
  Pepe_HttpResponseWrite(response, wsResponseKey);
  Pepe_HttpResponseWriteCString(response, "\r\n\r\n"); 

  return true;
}

void
Pepe_WebsocketClose(Pepe_HttpResponse *response, u16 status)
{
  Pepe_String msg;
  byte buf[4];
  buf[0] = 0;

  buf[0] |= 8;
  buf[0] |= 0x80;
  buf[1] = 2;

  printf("closing connection\n");

  status = PEPE_IS_BE() ? status : Pepe_U16SwapBytes(status);
  *(u16 *)(buf + 2) = status;
  
  msg.base = buf;
  msg.length = 4;

  Pepe_HttpResponseWrite(response, msg);
  Pepe_HttpResponseClose(response);
}

#endif
