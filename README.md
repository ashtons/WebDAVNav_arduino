# WebDAVNav_arduino
Basic WebDAV server for Arduino

Uses SDFat library from https://github.com/greiman/SdFat

Currently supports PROPFIND, GET, PUT and DELETE which allows a WebDAV client to browse folders on the SDCard, download, upload and delete files

Tested on EtherTen


#Configuration

Set the IP address to a valid address on your network


    byte ip[] = { 192, 168, 1, 177 };

By default the server is listening on port 80

#Connection

Mount the server using the http address

    http://192.168.1.177/
    
The is currently no authentication, so no username or password is required to connect

#Screenshots
<img src="https://raw.githubusercontent.com/ashtons/WebDAVNav_arduino/master/screenshots/Screen%20Shot%201.png" width="200" />
<img src="https://raw.githubusercontent.com/ashtons/WebDAVNav_arduino/master/screenshots/Screen%20Shot%202.png" width="200" />
<img src="https://raw.githubusercontent.com/ashtons/WebDAVNav_arduino/master/screenshots/Screen%20Shot%203.png" width="200" />
