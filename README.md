# FFIOCache
A cache item for ffmpeg.

# Usage
  * 1.Download this project or files.
  
  * 2.Copy files to your project.
  
  * 3.Modify your code of ffmpeg and rebuild it

    open libavformat/hls.c
    find open_url_keepalive in the function open_url()
    Add a condition in the front row,  <  !(s->flags & AVFMT_FLAG_CUSTOM_IO) > 
  
  * 4.Replace function.
    
    ---
    from:
    ```C
    AVFormatContext * fc = NULL;
    avformat_open_input(&fc, url, fmt,  opts);
    ```
    to:
    ```C
    AVFormatContext * fc = NULL;
    ff_io_cache_format_open_input(&fc, url, fmt,  opts, cache_flags);
    ```
    ---
    from:
    ```C
    avformat_free_context(fc);
    ```
    to:
    ```C
    ff_io_cache_format_free_context(fc);
    ```
    ---
    from:
    ```C
    avformat_close_input(fc);
    ```
    to:
    ```C
    ff_io_cache_format_close_input(fc);
    ```
    ---
