# FFIOCache
A cache item for ffmpeg.

# Usage
  * 1.Download this project or files.
  
  * 2.Copy files to your project.
  
  * 3.Modify your code of ffmpeg and rebuild it

    open libavformat/hls.c <br>
    find open_url_keepalive in the function open_url() <br>
    Add a condition in the front row:<br/>
   	if(!(s->flags & AVFMT_FLAG_CUSTOM_IO && ..... )  <br>
  
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

  * 5.Other

  	Must call ff_io_cache_global_set_path before any
# Prompt

	Don`t set AVFormatContext->opaque
	Don`t set AVFormatContext->io_open
	Don`t set AVFormatContext->io_close
# Contact
	helei0908@hotmail.com
