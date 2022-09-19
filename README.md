# cpp-grpc-server-example
Example of the C++ grpc simple stream server to test some hypothesis

## This branch is for testing the implementation of the checking when the client is goes offline (not listenings for a messages anymore).

You can build the server and client apps and run them in parallel, then close the client. The expected behavior is that the server will show the write error message (considering the clinet is offline) when the ping request from the parallel thread is failed.
