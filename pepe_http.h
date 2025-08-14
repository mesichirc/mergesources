#ifndef PEPE_HTTP_H
#define PEPE_HTTP_H

// needs #include <sys/socket.h> for accept, listen, recv, sockaddr, sockaddr_in  etc socket things
// needs #include <unistd.h> for fork
#include "pepe_core.h"
#include "pepe_encoding.h"
#include "pepe_memcmp.h"
#include "u.h"
#define PEPE_HTTP_WORKERS 4


typedef struct Pepe_HttpHeader Pepe_HttpHeader;
struct Pepe_HttpHeader {
  Pepe_String key;
  Pepe_String value;
};

typedef struct Pepe_HttpHeaderInterator Pepe_HttpHeaderInterator;
struct Pepe_HttpHeaderInterator {
  Pepe_String value;
};


typedef struct Pepe_HttpHeaders Pepe_HttpHeaders;
struct Pepe_HttpHeaders {
  Pepe_HttpHeader *data;
  u32             length;
  u32             capacity;
};

typedef struct Pepe_HttpResponse  Pepe_HttpResponse;
struct Pepe_HttpResponse {
  Pepe_Arena *arena;
  i32 connection;
};

#define PEPE_HTTP_REQUEST_SIZE 65536
#define PEPE_HTTP_RESPONSE_SIZE 65536

typedef PEPE_PACKED_ENUM {
  PEPE_HTTP_METHOD_UNKNOWN,
  PEPE_HTTP_METHOD_GET,
  PEPE_HTTP_METHOD_POST,
  PEPE_HTTP_METHOD_PUT,
  PEPE_HTTP_METHOD_PATCH,
  PEPE_HTTP_METHOD_DELETE
} Pepe_HttpMethod;

typedef struct Pepe_HttpRequest Pepe_HttpRequest;
struct Pepe_HttpRequest {
  Pepe_Slice memory;
  i32 connection;
  Pepe_HttpHeaders headers; 
  Pepe_String uri;
  Pepe_String body;
  Pepe_HttpMethod method;
  u8 valid;
};

typedef struct Pepe_HttpHandler Pepe_HttpHandler;
struct Pepe_HttpHandler {
  void (*handle)(Pepe_HttpRequest *request, Pepe_HttpResponse *response, void *userdata);
  void *userdata;
};

u16
Pepe_SwapBytesInPort(u16 port)
{
	return (port << 8) | ((port & 0xFF00) >> 8);
}

Pepe_HttpMethod
Pepe_HttpMethodByString(Pepe_String string)
{
  Pepe_HttpMethod method;
  method = PEPE_HTTP_METHOD_UNKNOWN;

  
  if (Pepe_StringCmp(string, Pepe_StringFromCString("POST"))) {
    method = PEPE_HTTP_METHOD_POST;
  } else if (Pepe_StringCmp(string, Pepe_StringFromCString("GET"))) {
    method = PEPE_HTTP_METHOD_GET;
  } else if (Pepe_StringCmp(string, Pepe_StringFromCString("PUT"))) {
    method = PEPE_HTTP_METHOD_PUT;
  } else if (Pepe_StringCmp(string, Pepe_StringFromCString("PATCH"))) {
    method = PEPE_HTTP_METHOD_PATCH; 
  } else if (Pepe_StringCmp(string, Pepe_StringFromCString("DELETE"))) {
    method = PEPE_HTTP_METHOD_DELETE;
  }

  return method;
}

Pepe_HttpHeader *
Pepe_HttpAddHeader(Pepe_Arena *arena, Pepe_HttpHeaders *headers)
{
  u32 capacity;
  if (headers->length >= headers->capacity) {
    capacity = headers->capacity * 2;
    PEPE_ARENA_RESIZE(arena, headers->data, headers->capacity, capacity);
    headers->capacity = capacity;
  }
  
  return &headers->data[headers->length++];
}

Pepe_String
Pepe_HttpMethodPrint(Pepe_HttpMethod method)
{
  Pepe_String string;
  switch (method) {
    case PEPE_HTTP_METHOD_GET: {
      string = Pepe_StringFromCString("GET");
      break;
    }
    case PEPE_HTTP_METHOD_POST: {
      string = Pepe_StringFromCString("POST");
      break;
    }
    case PEPE_HTTP_METHOD_PUT: {
      string = Pepe_StringFromCString("PUT");
      break;
    }
    case PEPE_HTTP_METHOD_PATCH: {
      string = Pepe_StringFromCString("PATCH");
      break;
    }
    case PEPE_HTTP_METHOD_DELETE: {
      string = Pepe_StringFromCString("DELETE");
      break;
    }
    case PEPE_HTTP_METHOD_UNKNOWN:
    default: {
      string = Pepe_StringFromCString("UNKNOWN");
      break;
    }

  }

  return string;
}



bool
Pepe_HttpHeaderInteratorHasNext(Pepe_HttpHeaderInterator *iterator)
{
  return iterator->value.length != 0;
}

Pepe_String
Pepe_HttpHeaderInteratorGetNext(Pepe_HttpHeaderInterator *iterator)
{
  i64 n;
  Pepe_Slice  currentSlice;
  Pepe_String currentString;
  Pepe_Slice  left, right;

  currentString = iterator->value;
  if (currentString.length == 0) {
    return currentString;
  }
  n = Pepe_StringToken(iterator->value, Pepe_StringFromCString(", "));
  if (n == -1) {
    iterator->value.length = 0;
    iterator->value.base = nil;
    return currentString;
  }

  currentSlice.length = iterator->value.length;
  currentSlice.capacity = iterator->value.length;
  currentSlice.base = iterator->value.base;

  left = Pepe_SliceLeft(currentSlice, n);  
  right = Pepe_SliceLeft(currentSlice, n + 2);  
  iterator->value = Pepe_StringInit(right);
  return Pepe_StringInit(left);
}

void
Pepe_HttpRequestPrint(Pepe_HttpRequest *request) 
{
  Pepe_String methodstr;
  Pepe_HttpHeader header;
  u32 i;
  printf("REQUEST:\n");
  printf("  Valid = %s\n", request->valid ? "YES" : "NO");
  printf("  Uri = %.*s\n", (u32)request->uri.length, request->uri.base);
  methodstr = Pepe_HttpMethodPrint(request->method);
  printf("  Method = %.*s\n", (u32)methodstr.length, methodstr.base);
  for (i = 0; i < request->headers.length; i++) {
    if (i == 0) {
      printf("  Headers:\n");
    }
    header = request->headers.data[i];
    printf("    %.*s: %.*s\n", (u32)header.key.length, header.key.base, (u32)header.value.length, header.value.base);
  }
  if (request->body.length > 0) {
    printf("  Body = %.*s\n", (u32)request->body.length, request->body.base);
  }
}

Pepe_String
Pepe_HttpHeadersGet(Pepe_HttpHeaders *headers, Pepe_String key) 
{
  Pepe_String result;
  u32 i;

  result.base = nil;
  result.length = 0;

  for (i = 0; i < headers->length; i++) {
    if (Pepe_IsStringsEqualCaseInsensitive(headers->data[i].key, key)) {
      result = headers->data[i].value;
      break;
    }
  }

  return result;
}

bool
Pepe_HttpHeadersHasValue(Pepe_HttpHeaders *headers, Pepe_String key, Pepe_String expectedValue)
{
  Pepe_HttpHeaderInterator iterator;
  bool found;

  found = false;
  iterator.value = Pepe_HttpHeadersGet(headers, key);
  for (;Pepe_HttpHeaderInteratorHasNext(&iterator);) {
    if (Pepe_IsStringsEqualCaseInsensitive(Pepe_HttpHeaderInteratorGetNext(&iterator), expectedValue)) {
      found = true;
      break;
    }
  } 

  return found; 
}

bool
Pepe_HttpHeadersHasValueCString(Pepe_HttpHeaders *headers, char *key, char *expectedValue)
{
  return Pepe_HttpHeadersHasValue(headers, Pepe_StringFromCString(key), Pepe_StringFromCString(expectedValue));
}

void
Pepe_HttpParseRequest(Pepe_HttpRequest *request, Pepe_Arena *arena)
{
  Pepe_Slice currentSlice;
  Pepe_String SPACE;
  Pepe_String CRLF;
  Pepe_String HEAD_DELIM;
  Pepe_HttpHeader *header;
  i64 n;

  SPACE = Pepe_StringFromCString(" ");
  CRLF  = Pepe_StringFromCString("\r\n");
  HEAD_DELIM = Pepe_StringFromCString(": ");

  n = 0;
  currentSlice = request->memory;

  n = Pepe_SliceToken(currentSlice, SPACE);
  if (n < 0) {
    request->valid = 0;
    return;
  }

  request->method = Pepe_HttpMethodByString(Pepe_StringInit(Pepe_SliceLeft(currentSlice, n)));
  if (request->method == PEPE_HTTP_METHOD_UNKNOWN || n == (i64)currentSlice.length) {
    request->valid = 0;
    return;
  }

  currentSlice = Pepe_SliceRight(currentSlice, n + SPACE.length);

  n = Pepe_SliceToken(currentSlice, SPACE);

  if (n < 0) {
    request->valid = 0;
    return;
  }

  request->uri = Pepe_StringInit(Pepe_SliceLeft(currentSlice, n));
  currentSlice = Pepe_SliceRight(currentSlice, n + SPACE.length);

  n = Pepe_SliceToken(currentSlice, CRLF);

  if (n < 0) {
    request->valid = 0;
    return;
  }

  currentSlice = Pepe_SliceRight(currentSlice, n + CRLF.length);
  if (currentSlice.length == 2) {
    if (((byte *)currentSlice.base)[0] == '\r' && ((byte *)currentSlice.base)[1] == '\n') {
      request->valid = 1;
    } else {
      request->valid = 0;
    }
    return;
  } 

  // we don't have headers, but have body
  if (((byte *)currentSlice.base)[0] == '\r' && ((byte *)currentSlice.base)[1] == '\n') {
    currentSlice = Pepe_SliceRight(currentSlice, 2);
    n = Pepe_SliceToken(currentSlice, CRLF);
    // body must end with \r\n
    if (n >= 0) {
      request->body = Pepe_StringInit(Pepe_SliceLeft(currentSlice, n));
      request->valid = 1;
    } else {
      request->valid = 0;
    }
    return;
  } 

  request->headers.capacity = 8;
  request->headers.data = PEPE_ARENA_ALLOC(arena, sizeof(Pepe_HttpHeader) * request->headers.capacity);
  request->headers.length = 0;
  
  // we have headers lets parse it
  for (;;) {
    if (currentSlice.length < 2) {
      request->valid = 0;
      return;
    }
    if (((byte *)currentSlice.base)[0] == '\r' && ((byte *)currentSlice.base)[1] == '\n') {
      break;
    }

    header = Pepe_HttpAddHeader(arena, &request->headers);

    n = Pepe_SliceToken(currentSlice, HEAD_DELIM);
    if (n < 0) {
      request->valid = 0;
      return;
    } else {
      header->key = Pepe_StringInit(Pepe_SliceLeft(currentSlice, n));
      currentSlice = Pepe_SliceRight(currentSlice, n + HEAD_DELIM.length);
    }

    n = Pepe_SliceToken(currentSlice, CRLF);
    if (n < 0) {
      request->valid = 0;
      return;
    } else {
      header->value = Pepe_StringInit(Pepe_SliceLeft(currentSlice, n));
      currentSlice = Pepe_SliceRight(currentSlice, n + CRLF.length);
    }
  }

  request->body.length = 0;
  request->body.base = nil;

  // we don't have body so return
  if (currentSlice.length == 2) {
    if (((byte *)currentSlice.base)[0] == '\r' && ((byte *)currentSlice.base)[1] == '\n') {
      request->valid = 1;
    } else {
      request->valid = 0;
    }
    return; 
  }


  if (((byte *)currentSlice.base)[currentSlice.length - 2] == '\r' && ((byte *)currentSlice.base)[currentSlice.length - 1] == '\n') {
    request->body = Pepe_StringInit(Pepe_SliceLeft(currentSlice, currentSlice.length - 2));
    request->valid = 1;
  } else {
    request->valid = 0;
  }
} 


bool
Pepe_HttpResponseWrite(Pepe_HttpResponse *response, Pepe_String data)
{
  return write(response->connection, data.base, data.length) >= 0;
}

bool
Pepe_HttpResponseWriteCString(Pepe_HttpResponse *response, char *string)
{
  return Pepe_HttpResponseWrite(response, Pepe_StringFromCString(string));
}

bool
Pepe_HttpResponseClose(Pepe_HttpResponse *response)
{
  return close(response->connection);
}

void
Pepe_HttpRespond(Pepe_Arena *arena, i32 s, Pepe_HttpHandler handler)
{
  Pepe_HttpRequest request;
  Pepe_HttpResponse response;
  Pepe_Slice requestBuffer;
  i32 rcvd;
  
  request.connection = s;
  response.connection = s;
  response.arena = arena;

  requestBuffer.length = 0;
  requestBuffer.capacity = PEPE_HTTP_REQUEST_SIZE;
  requestBuffer.base = PEPE_ARENA_ALLOC(arena, requestBuffer.capacity);

  assert(requestBuffer.base);

  rcvd = recv(s, requestBuffer.base, requestBuffer.capacity, 0);
  if (rcvd < 0) {
    perror("recv() call failed");
  } else if (rcvd == 0) {
    fprintf(stderr, "client disconnected upexpectedly.\n");
  } else {
    requestBuffer.length = (u64)rcvd;
    request.memory = requestBuffer;
    Pepe_HttpParseRequest(&request, arena);
    if (request.valid) {
      handler.handle(&request, &response, handler.userdata);
    } else {
      fprintf(stderr, "can't parse request.\n");
    }
  }
}

void
Pepe_HttpWorker(Pepe_Arena *arena, i32 l, Pepe_HttpHandler handler)
{
  struct sockaddr_in clientAddr;
  socklen_t addrLen;

  i32 s;
  addrLen = sizeof(clientAddr);
  for (;;) {
    s = accept(l, (struct sockaddr *) &clientAddr, &addrLen);
    if (s < 0) {
      perror("accept() call failed");
      continue;
    }
    PEPE_ARENA_CLEAR(arena);
    Pepe_HttpRespond(arena, s, handler);
  } 
}

typedef struct Pepe_HttpWorkers Pepe_HttpWorkers;
struct Pepe_HttpWorkers {
  i32 *data;
  u32 length;
};

void
Pepe_HttpListenAndServe(Pepe_Arena arena, Pepe_HttpHandler handler, u16 port, Pepe_HttpWorkers workers)
{
  struct sockaddr_in addr;
	i32	enable = 1;
	i32	l, i, pid;

  printf("start listening on port %d\n", port);

  l = socket(PF_INET, SOCK_STREAM, 0);
  if (l < 0) {
    perror("socket() call failed");
    return;
  }

  if (setsockopt(l, SOL_SOCKET, SO_REUSEPORT, &enable, sizeof(enable)) < 0) {
    perror("setsockopt() call failed");
    return;
  }

  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = PEPE_IS_BE() ? port : Pepe_SwapBytesInPort(port);
  if (bind(l, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    perror("bind() call failed");
    return;
  }

  if (listen(l, 128) < 0) {
    perror("listen() call failed");
    return;
  }
  for (i = 0; i < (i32)workers.length - 1; i++) {
    pid = fork();
    workers.data[i] = pid;
    if (pid == 0) {
      break;
    }
  }
  if (pid == 0) {
    Pepe_HttpWorker(&arena, l, handler);
  }

  return;
}

i32
Pepe_ReadFromConnection(Pepe_HttpRequest *request, Pepe_Slice buffer)
{
  assert(buffer.base);
  return recv(request->connection, buffer.base, buffer.length, 0);
}

Pepe_String
Pepe_WebsocketSecWebsocketAcceptHeader(Pepe_Arena *arena, Pepe_String clientKey)
{
  byte       sha1Buffer[21];
  Pepe_String wsKeySalt;
  Pepe_Slice hashMemory;
  Pepe_String wsKey;

  hashMemory.base = PEPE_ARENA_ALLOC(arena, PEPE_SHA1_BASE64_LENGTH);
  hashMemory.length = hashMemory.capacity = PEPE_SHA1_BASE64_LENGTH;
  wsKeySalt = Pepe_StringFromCString("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");

  wsKey.length = clientKey.length + wsKeySalt.length;
  wsKey.base   = PEPE_ARENA_ALLOC(arena, wsKey.length);
  assert(hashMemory.base && wsKey.base);
  memmove(wsKey.base, clientKey.base, clientKey.length);
  memmove(wsKey.base + clientKey.length, wsKeySalt.base, wsKeySalt.length);

  Pepe_SHA1(sha1Buffer, wsKey);
  wsKey.base = sha1Buffer;
  wsKey.length = 20;

  return Pepe_Base64Encode(wsKey, hashMemory);
}

#endif
