# animal3D-SDK-202101SP
animal3D SDK and related course frameworks for spring 2021.
Project 1:
To run the project, make sure the correct IP and port for the server machine are in the config file (in the resources folder) before using the remote VS launcher.
Launch both the GPRO-NET-VSLAUNCH and GPRO-NET-VSLAUNCH-REMOTE as Administrator for the project to fully work.
First run the GPRO-NET-VSLAUNCH-REMOTE project, it should connect to the remote machine if the steps above are completed.
Then run some instances of the GPRO-NET-VSLAUNCH project, at least 2, to test sending and recieving messages.
On the client(s), a username is required to connect to the server and send messages.
Two commands for the client exist; /getUsers returns a list of all users, and /update updates the user with messages that have been sent.
The client will be updated with new messages if they either use the /update command or send a message themselves.
A log of messages sent, with a timestamp and username stamp, is written to a text file located on the Z drive (if available).