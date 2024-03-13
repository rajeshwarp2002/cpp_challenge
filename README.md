How to run
=============
copy all the files from this repo to any folder.

docker build -t processor_image .
docker run --rm -v /:/input_dir processor_image /input_dir/input.txt

This appends the logs every minute to file /input.txt on local machine. The logs look as follows
Timestamp: Wed Mar 13 00:28:53 2024
URL: /profile/smadigan, Response Code: 404, Count: 533
URL: /profile/maxidelo, Response Code: 404, Count: 532
URL: /feed/aambertin, Response Code: 201, Count: 499

Approach:
==========
The trick here to match request and response is to use "X-Trace-ID" header. This is common in request and response. So build a transaction map that is keyed by Trace-ID and on response delete the entry from map.
This map stores the URL. On response, get URL from this map and store in urlMap which is keyed by URL and status code(status code extracted from response)

Some food for thought (a.k.a., nice to have's)
==============
What would happen if we added messages in another protocol / format?

==>Those messages will be ignored. And may be counted in real world.

What would happen if the message rate exceeded your CPU capacity?

==> the messages starts getting dropped when pipe is full and processor is slow in consuming because its already running at 100% CPU 

What would happen if some messages never got a response?

==> the transaction table which holds temporary state need to timeout the requests, so that we dont leak memory in transaction table.
