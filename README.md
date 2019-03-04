mmap api experiment
===================
This example shows that we can creat different mapping point to same region. Page one is just like `10101010...`. And page two is `0202020202...`. After copying and remapping, page two point to the file location of page one. All content in page two now survive on page one.
