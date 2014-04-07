#  
#  To use this DEPS file to re-create a Chromium release you
#  need the tools from http://dev.chromium.org installed.
#  
#  This DEPS file corresponds to Chromium 4.0.221.8
#  
#  
#  
deps_os = {
   'win': {
      'src/third_party/xulrunner-sdk':
         '/trunk/deps/third_party/xulrunner-sdk@17887',
      'src/chrome_frame/tools/test/reference_build/chrome':
         '/trunk/deps/reference_builds/chrome_frame@27181',
      'src/third_party/cygwin':
         '/trunk/deps/third_party/cygwin@11984',
      'src/third_party/pthreads-win32':
         '/trunk/deps/third_party/pthreads-win32@26716',
      'src/third_party/python_24':
         '/trunk/deps/third_party/python_24@22967',
      'src/third_party/ffmpeg/binaries/chromium/win/ia32':
         '/trunk/deps/third_party/ffmpeg/binaries/win@27457',
   },
   'mac': {
      'src/third_party/GTM':
         'http://google-toolbox-for-mac.googlecode.com/svn/trunk@223',
      'src/third_party/pdfsqueeze':
         'http://pdfsqueeze.googlecode.com/svn/trunk@2',
      'src/third_party/WebKit/WebKit/mac':
         'http://svn.webkit.org/repository/webkit/trunk/WebKit/mac@49111',
      'src/third_party/WebKit/WebKitLibraries':
         'http://svn.webkit.org/repository/webkit/trunk/WebKitLibraries@49111',
      'src/third_party/ffmpeg/binaries/chromium/mac/ia32_dbg':
         '/trunk/deps/third_party/ffmpeg/binaries/mac_dbg@27457',
      'src/third_party/ffmpeg/binaries/chromium/mac/ia32':
         '/trunk/deps/third_party/ffmpeg/binaries/mac@27457',
   },
   'unix': {
      'src/third_party/xdg-utils':
         '/trunk/deps/third_party/xdg-utils@26314',
      'src/third_party/ffmpeg/binaries/chromium/linux/x64_dbg':
         '/trunk/deps/third_party/ffmpeg/binaries/linux_64_dbg@27457',
      'src/third_party/ffmpeg/binaries/chromium/linux/ia32':
         '/trunk/deps/third_party/ffmpeg/binaries/linux@27457',
      'src/third_party/ffmpeg/binaries/chromium/linux/ia32_dbg':
         '/trunk/deps/third_party/ffmpeg/binaries/linux_dbg@27457',
      'src/third_party/ffmpeg/binaries/chromium/linux/x64':
         '/trunk/deps/third_party/ffmpeg/binaries/linux_64@27457',
   },
}

deps = {
   'src/chrome/test/data/layout_tests/LayoutTests/fast/events':
      'http://svn.webkit.org/repository/webkit/trunk/LayoutTests/fast/events@49111',
   'src/third_party/WebKit/WebKit/chromium':
      'http://svn.webkit.org/repository/webkit/trunk/WebKit/chromium@49111',
   'src/third_party/WebKit/WebCore':
      'http://svn.webkit.org/repository/webkit/trunk/WebCore@49111',
   'src/breakpad/src':
      'http://google-breakpad.googlecode.com/svn/trunk/src@402',
   'src/chrome/test/data/layout_tests/LayoutTests/fast/workers':
      'http://svn.webkit.org/repository/webkit/trunk/LayoutTests/fast/workers@49111',
   'src/third_party/tcmalloc/tcmalloc':
      'http://google-perftools.googlecode.com/svn/trunk@74',
   'src/googleurl':
      'http://google-url.googlecode.com/svn/trunk@119',
   'src/chrome/test/data/layout_tests/LayoutTests/http/tests/workers':
      'http://svn.webkit.org/repository/webkit/trunk/LayoutTests/http/tests/workers@49111',
   'src/third_party/protobuf2/src':
      'http://protobuf.googlecode.com/svn/trunk@219',
   'src/native_client':
      'http://nativeclient.googlecode.com/svn/trunk/src/native_client@819',
   'src/chrome/test/data/layout_tests/LayoutTests/http/tests/xmlhttprequest':
      'http://svn.webkit.org/repository/webkit/trunk/LayoutTests/http/tests/xmlhttprequest@49111',
   'src/tools/gyp':
      'http://gyp.googlecode.com/svn/trunk@671',
   'src/sdch/open-vcdiff':
      'http://open-vcdiff.googlecode.com/svn/trunk@26',
   'src/third_party/WebKit/JavaScriptCore':
      'http://svn.webkit.org/repository/webkit/trunk/JavaScriptCore@49111',
   'src/chrome/test/data/layout_tests/LayoutTests/fast/js/resources':
      'http://svn.webkit.org/repository/webkit/trunk/LayoutTests/fast/js/resources@49111',
   'src/chrome/test/data/layout_tests/LayoutTests/http/tests/resources':
      'http://svn.webkit.org/repository/webkit/trunk/LayoutTests/http/tests/resources@49111',
   'src/tools/page_cycler/acid3':
      '/trunk/deps/page_cycler/acid3@19546',
   'src/build/util/support':
      '/trunk/deps/support@28156',
   'src/chrome/test/data/layout_tests/LayoutTests/storage/domstorage':
      'http://svn.webkit.org/repository/webkit/trunk/LayoutTests/storage/domstorage@49111',
   'src/chrome/tools/test/reference_build':
      '/trunk/deps/reference_builds@25513',
   'src/v8':
      'http://v8.googlecode.com/svn/trunk@2997',
   'src/third_party/WebKit/LayoutTests':
      'http://svn.webkit.org/repository/webkit/trunk/LayoutTests@49111',
   'src':
      '/branches/221/src@28108',
   'src/third_party/icu':
      '/trunk/deps/third_party/icu42@27687',
   'src/third_party/WebKit':
      '/trunk/deps/third_party/WebKit@27313',
   'src/third_party/hunspell':
      '/trunk/deps/third_party/hunspell128@27726',
   'src/testing/gtest':
      'http://googletest.googlecode.com/svn/trunk@329',
   'src/third_party/skia':
      'http://skia.googlecode.com/svn/trunk@364',
}

skip_child_includes =  ['breakpad', 'chrome_frame', 'gears', 'native_client', 'o3d', 'sdch', 'skia', 'testing', 'third_party', 'v8'] 

hooks =  [{'action': ['python', 'src/build/gyp_chromium'], 'pattern': '.'}, {'action': ['python', 'src/build/win/clobber_generated_headers.py', '$matching_files'], 'pattern': '\\.grd$'}, {'action': ['python', 'src/build/mac/clobber_generated_headers.py'], 'pattern': '.'}] 

include_rules =  ['+base', '+build', '+ipc', '+unicode', '+testing', '+webkit/port/platform/graphics/skia/public']