# HTTP/1.1 RFCs

1. **HTTP/1.1 Basics:**
   - The core specification for HTTP/1.1 is defined in [RFC 7230](https://tools.ietf.org/html/rfc7230): "Hypertext Transfer Protocol (HTTP/1.1): Message Syntax and Routing." This RFC outlines the basic syntax and structure of HTTP/1.1 messages.

2. **Request Methods:**
   - Information about HTTP request methods (GET, POST, DELETE) is covered in [RFC 7231](https://datatracker.ietf.org/doc/html/rfc7231#section-4): "Hypertext Transfer Protocol (HTTP/1.1): Semantics and Content." This RFC provides details on the semantics and meaning of various HTTP methods.

3. **Headers:**
   - The details about HTTP headers can be found in [RFC 7230](https://tools.ietf.org/html/rfc7230) and [RFC 7231](https://tools.ietf.org/html/rfc7231). These RFCs cover the message header fields and how they are used in requests and responses.

4. **Status Codes:**
   - HTTP status codes are defined in [RFC 7231](https://tools.ietf.org/html/rfc7231). This RFC provides a detailed list of status codes and their meanings.

# Request

# GET

In an HTTP/1.1 GET request, the only mandatory header is the `Host` header. The Host header is required in HTTP/1.1 requests and specifies the domain name of the server (and optionally the port number) to which the request is being sent.

Other headers, including the `User-Agent` and `Accept` headers, are optional in the context of a basic GET request. However, they are commonly included for additional information or to provide preferences to the server.

So, for a basic HTTP/1.1 GET request, the following is required:

`Host`: Specifies the domain name of the server.
And the following headers are optional but commonly used:

`User-Agent`: Identifies the user agent (browser or other client) initiating the request.

`Accept`: Informs the server about the types of media that the client can process.

```HTTP 1.1
GET /path/to/resource HTTP/1.1
Host: example.com
Accept: text/html, application/xhtml+xml
```

When making an HTTP request, various errors may occur, and the server can respond with corresponding status codes to indicate the nature of the issue. Here are some potential errors that could occur with the given example request, along with the likely HTTP status codes for each:

1. **404 Not Found:**
   - If the requested resource at `/path/to/resource` does not exist on the server, the server will respond with a "404 Not Found" status code.

2. **400 Bad Request:**
   - If the request is malformed or lacks required information, the server may respond with a "400 Bad Request" status code. For instance, if the `Host` header is missing or incorrectly formatted, the server might reject the request.

3. **406 Not Acceptable:**
   - If the server cannot provide a response that matches the client's preferences specified in the `Accept` header, it may respond with a "406 Not Acceptable" status code.

4. **401 Unauthorized:**
   - If the requested resource requires authentication, and the client fails to provide valid credentials in the `Authorization` header, the server may respond with a "401 Unauthorized" status code.

These are just examples, and the actual response codes can vary depending on the server's configuration and the specific circumstances. Always refer to the server's documentation or check the response from the server for accurate information about the encountered error.

`Host`: Specifies the domain name of the server (and optionally the port number) to which the request is being sent. This header is required in HTTP/1.1 requests.

## Post

certain types of requests may require additional headers for proper processing, such as Content-Type and Content-Length for POST requests with a message body.

# Request Line

The request line is a mandatory component of an HTTP request, and it is positioned as the initial line in the request message. It serves as a fundamental element, providing essential details about the client's request to the server.

## Structure

The syntax of the request line adheres to the following format:

```
<HTTP Method> <Request-URI> <HTTP Version>
```

- `<HTTP Method>`: Denotes the HTTP method or verb, specifying the type of request being made (e.g., GET, POST, DELETE).

- `<Request-URI>`: Represents the Uniform Resource Identifier (URI) or path that points to the desired resource on the server.

- `<HTTP Version>`: Indicates the version of the HTTP protocol being used for the request (e.g., HTTP/1.1).

## Example

For instance, in the request `GET /index.html HTTP/1.1`, the request line consists of:

```
GET /index.html HTTP/1.1
```

This structured line is imperative for the server's proper interpretation of the client's intent and forms the introductory element of an HTTP request. It must consistently be the first line in the request message.

# Invalid HTTP Method Response

When an HTTP request includes an invalid or unsupported method, the server is expected to respond with an appropriate HTTP status code and potentially additional information to indicate the nature of the error.

## HTTP Status Codes

The following HTTP status code is commonly used to signify errors related to invalid HTTP methods:

- **405 Method Not Allowed:**
  - The server responds with a "405 Method Not Allowed" status code when the request method is not supported for the requested resource.

## Example Response

If a client sends a request with an invalid HTTP method:

```
INVALID /index.html HTTP/1.1
```

The server may respond with a "405 Method Not Allowed" status code:

```
HTTP/1.1 405 Method Not Allowed
Allow: GET, POST, DELETE
Content-Type: text/html

<!DOCTYPE html>
<html>
<head>
    <title>405 Method Not Allowed</title>
</head>
<body>
    <h1>Method Not Allowed</h1>
    <p>The specified method is not allowed for the requested resource.</p>
</body>
</html>
```

This response includes an "Allow" header indicating the allowed methods for the requested resource and provides information to the client about the reason for the error.

# Invalid HTTP Version Response

When an HTTP request specifies an unsupported or incorrect HTTP version, the server is expected to respond with an appropriate HTTP status code to communicate the issue to the client.

## HTTP Status Codes

The following HTTP status codes are commonly used to signify errors related to the HTTP version:

- **505 HTTP Version Not Supported:**
  - The server responds with a "505 HTTP Version Not Supported" status code when it does not support the specified HTTP version.

## Example Response

If a client sends a request with an unsupported HTTP version:

```
GET /index.html HTTP/1.2
```

The server may respond with a "505 HTTP Version Not Supported" status code:

```
HTTP/1.1 505 HTTP Version Not Supported
Content-Type: text/html

<!DOCTYPE html>
<html>
<head>
    <title>505 HTTP Version Not Supported</title>
</head>
<body>
    <h1>HTTP Version Not Supported</h1>
    <p>The server does not support the specified HTTP version.</p>
</body>
</html>
```

This response includes a brief HTML representation indicating that the HTTP version is not supported, providing clarity to the client about the reason for the error.

# HTTP Request Validation Cases

When processing an HTTP request, several potential cases of errors related to the request line, method, URI, and HTTP version can occur. The following documentation outlines various scenarios and their corresponding HTTP status codes and minimal response body text.

## 1. Invalid HTTP Method

### Request:
```
INVALIDMETHOD /index.html HTTP/1.1
```

### Response:
```
HTTP/1.1 405 Method Not Allowed
```

## 2. Unsupported HTTP Version

### Request:
```
GET /index.html HTTP/1.2
```

### Response:
```
HTTP/1.1 505 HTTP Version Not Supported
```

## 3. Invalid URI

### Request:
```
GET /index.html?invalid HTTP/1.1
```

### Response:
```
HTTP/1.1 400 Bad Request
```

## 4. Missing Request Line Components

### Request:
```
GET HTTP/1.1
```

### Response:
```
HTTP/1.1 400 Bad Request
```

## 5. Empty Request Line

### Request:
```
```

### Response:
```
HTTP/1.1 400 Bad Request
```

## 6. Unknown Protocol

### Request:
```
UNKNOWN /index.html HTTP/1.1
```

### Response:
```
HTTP/1.1 400 Bad Request
```

## 7. Invalid Spaces in Request Line

### Request:
```
GET /index.html HTTP/1.1
```

### Response:
```
HTTP/1.1 400 Bad Request
```

## 8. Invalid Character in HTTP Version

### Request:
```
GET /index.html HTTP/1_1
```

### Response:
```
HTTP/1.1 400 Bad Request
```

## 9. Invalid Character in URI

### Request:
```
GET /index.html@ HTTP/1.1
```

### Response:
```
HTTP/1.1 400 Bad Request
```

These cases cover potential errors related to the request line in an HTTP request. The corresponding HTTP status codes and concise response body text provide clear indications of the encountered issues.

# Invalid Characters in URI

The Uniform Resource Identifier (URI) is a standardized means of identifying resources. To maintain compatibility and conformity with URI specifications, certain characters are considered invalid and must be percent-encoded when used in a URI.

## Reserved Characters

The following characters are reserved and have special meanings in a URI. If they appear in a different context or without proper encoding, they can lead to ambiguity:

- `!`
- `#`
- `$`
- `&`
- `'`
- `(`
- `)`
- `*`
- `+`
- `,`
- `/`
- `:`
- `;`
- `=`
- `?`
- `@`

## Unsafe Characters

Additionally, the following characters are considered unsafe if they are used in a URI without proper encoding:

- `<`
- `>`
- `#`
- `%`
- `"`
- `{`
- `}`
- `|`
- `\`
- `^`
- `[`
- `]`

## Control Characters

Control characters and non-ASCII characters are generally not allowed in a URI without proper encoding.

## Percent-Encoding

To include reserved or unsafe characters in a URI, they must be percent-encoded. Percent-encoding involves representing the character with a percent sign ("%") followed by two hexadecimal digits corresponding to the ASCII code of the character.

For example, space (``) is represented as `%20`, and the exclamation mark (`!`) is represented as `%21`.

Understanding and adhering to these rules help ensure that URIs are correctly interpreted and processed across different systems and applications.

# Resouces

[How nginx processes a request](https://nginx.org/en/docs/http/request_processing.html)

