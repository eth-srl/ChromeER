#  
#  To use this DEPS file to re-create a Chromium release you
#  need the tools from http://dev.chromium.org installed.
#  
#  This DEPS file corresponds to Chromium 21.0.1166.1
#  
#  
#  
vars =  {'webkit_trunk': 'http://svn.webkit.org/repository/webkit/trunk'} 

deps_os = {
   'win': {
      'src/third_party/yasm/binaries':
      '/trunk/deps/third_party/yasm/binaries@74228',
      'src/third_party/nacl_sdk_binaries':
      '/trunk/deps/third_party/nacl_sdk_binaries@111576',
      'src/third_party/pefile':
      'http://pefile.googlecode.com/svn/trunk@63',
      'src/third_party/swig/win':
      '/trunk/deps/third_party/swig/win@69281',
      'src/third_party/lighttpd':
      '/trunk/deps/third_party/lighttpd@33727',
      'src/chrome/tools/test/reference_build/chrome_win':
      '/trunk/deps/reference_builds/chrome_win@137747',
      'src/rlz':
      'http://rlz.googlecode.com/svn/trunk@130',
      'src/third_party/xulrunner-sdk':
      '/trunk/deps/third_party/xulrunner-sdk@119756',
      'src/third_party/gnu_binutils':
      'http://src.chromium.org/native_client/trunk/deps/third_party/gnu_binutils@8673',
      'src/third_party/psyco_win32':
      '/trunk/deps/third_party/psyco_win32@79861',
      'src/third_party/mingw-w64/mingw/bin':
      'http://src.chromium.org/native_client/trunk/deps/third_party/mingw-w64/mingw/bin@8673',
      'src/chrome_frame/tools/test/reference_build/chrome_win':
      '/trunk/deps/reference_builds/chrome_win@89574',
      'src/third_party/cygwin':
      '/trunk/deps/third_party/cygwin@133786',
      'src/third_party/python_26':
      '/trunk/tools/third_party/python_26@89111',
      'src/third_party/syzygy/binaries':
      'http://sawbuck.googlecode.com/svn/trunk/syzygy/binaries@782',
      'src/third_party/nss':
      '/trunk/deps/third_party/nss@138140',
   },
   'mac': {
      'src/rlz':
      'http://rlz.googlecode.com/svn/trunk@130',
      'src/third_party/GTM':
      'http://google-toolbox-for-mac.googlecode.com/svn/trunk@516',
      'src/third_party/pdfsqueeze':
      'http://pdfsqueeze.googlecode.com/svn/trunk@4',
      'src/chrome/installer/mac/third_party/xz/xz':
      '/trunk/deps/third_party/xz@87706',
      'src/third_party/swig/mac':
      '/trunk/deps/third_party/swig/mac@69281',
      'src/chrome/tools/test/reference_build/chrome_mac':
      '/trunk/deps/reference_builds/chrome_mac@137727',
      'src/third_party/lighttpd':
      '/trunk/deps/third_party/lighttpd@33737',
      'src/third_party/nss':
      '/trunk/deps/third_party/nss@138140',
   },
   'unix': {
      'src/third_party/gold':
      '/trunk/deps/third_party/gold@124239',
      'src/third_party/swig/linux':
      '/trunk/deps/third_party/swig/linux@69281',
      'src/third_party/WebKit/Tools/gdb':
      Var("webkit_trunk")[:-6] + '/trunk/Tools/gdb@119581',
      'src/third_party/cros':
      'http://git.chromium.org/chromiumos/platform/cros.git@377f51d8',
      'src/third_party/xdg-utils':
      '/trunk/deps/third_party/xdg-utils@93299',
      'build/xvfb':
      '/trunk/tools/xvfb@121100',
      'src/third_party/openssl':
      '/trunk/deps/third_party/openssl@130472',
      'build/third_party/xvfb':
      '/trunk/tools/third_party/xvfb@125214',
      'build/third_party/cbuildbot_chromite':
      'https://git.chromium.org/chromiumos/chromite.git',
      'src/third_party/lss':
      'http://linux-syscall-support.googlecode.com/svn/trunk/lss@9',
      'src/third_party/cros_system_api':
      'http://git.chromium.org/chromiumos/platform/system_api.git@04134b25',
      'src/chrome/tools/test/reference_build/chrome_linux':
      '/trunk/deps/reference_builds/chrome_linux@137712',
   },
   'android': {
      'src/third_party/aosp':
      '/trunk/deps/third_party/aosp@122156',
      'src/third_party/freetype':
      'http://git.chromium.org/git/chromium/src/third_party/freetype.git@1f74e4e7ad3ca4163b4578fc30da26a165dd55e7',
   },
}

deps = {
   'src/third_party/mozc/chrome/chromeos/renderer':
      'http://mozc.googlecode.com/svn/trunk/src/chrome/chromeos/renderer@83',
   'src/content/test/data/layout_tests/LayoutTests/fast/filesystem/resources':
      Var("webkit_trunk")[:-6] + '/trunk/LayoutTests/fast/filesystem/resources@119581',
   'src/third_party/skia/include':
      'http://skia.googlecode.com/svn/trunk/include@4156',
   'build/scripts/private/data/reliability':
      '/trunk/src/chrome/test/data/reliability@109518',
   'src/third_party/flac':
      '/trunk/deps/third_party/flac@120197',
   'chromeos':
      '/trunk/src/tools/cros.DEPS@139701',
   'src/chrome/test/data/perf/frame_rate/content':
      '/trunk/deps/frame_rate/content@93671',
   'src/third_party/ots':
      'http://ots.googlecode.com/svn/trunk@87',
   'src/third_party/sfntly/cpp/src':
      'http://sfntly.googlecode.com/svn/trunk/cpp/src@128',
   'src/third_party/undoview':
      '/trunk/deps/third_party/undoview@119694',
   'src/googleurl':
      'http://google-url.googlecode.com/svn/trunk@175',
   'build/third_party/lighttpd':
      '/trunk/deps/third_party/lighttpd@58968',
   'src/third_party/webgl_conformance':
      '/trunk/deps/third_party/webgl/sdk/tests@138171',
   'src/third_party/hunspell_dictionaries':
      '/trunk/deps/third_party/hunspell_dictionaries@138928',
   'src/content/test/data/layout_tests/LayoutTests/storage/indexeddb':
      Var("webkit_trunk")[:-6] + '/trunk/LayoutTests/storage/indexeddb@119581',
   'src/content/test/data/layout_tests/LayoutTests/media':
      Var("webkit_trunk")[:-6] + '/trunk/LayoutTests/media@119581',
   'src/content/test/data/layout_tests/LayoutTests/http/tests/appcache':
      Var("webkit_trunk")[:-6] + '/trunk/LayoutTests/http/tests/appcache@119581',
   'commit-queue':
      '/trunk/tools/commit-queue@139712',
   'src/third_party/scons-2.0.1':
      'http://src.chromium.org/native_client/trunk/src/third_party/scons-2.0.1@8673',
   'src/third_party/webdriver/pylib':
      'http://selenium.googlecode.com/svn/trunk/py@13487',
   'build/scripts/gsd_generate_index':
      '/trunk/tools/gsd_generate_index@110568',
   'src':
      '/branches/1166/src@140874',
   'src/third_party/libyuv':
      'http://libyuv.googlecode.com/svn/trunk@247',
   'src/third_party/hunspell':
      '/trunk/deps/third_party/hunspell@132738',
   'src/third_party/libphonenumber/src/phonenumbers':
      'http://libphonenumber.googlecode.com/svn/trunk/cpp/src/phonenumbers@456',
   'src/third_party/libphonenumber/src/resources':
      'http://libphonenumber.googlecode.com/svn/trunk/resources@456',
   'src/third_party/safe_browsing/testing':
      'http://google-safe-browsing.googlecode.com/svn/trunk/testing@110',
   'src/content/test/data/layout_tests/LayoutTests/http/tests/workers':
      Var("webkit_trunk")[:-6] + '/trunk/LayoutTests/http/tests/workers@119581',
   'src/content/test/data/layout_tests/LayoutTests/fast/workers':
      Var("webkit_trunk")[:-6] + '/trunk/LayoutTests/fast/workers@119581',
   'src/third_party/WebKit/Source':
      Var("webkit_trunk")[:-6] + '/trunk/Source@119581',
   'build/third_party/gsutil':
      'http://gsutil.googlecode.com/svn/trunk/src@145',
   'src/third_party/pyftpdlib/src':
      'http://pyftpdlib.googlecode.com/svn/trunk@977',
   'src/third_party/WebKit/LayoutTests':
      Var("webkit_trunk")[:-6] + '/trunk/LayoutTests@119581',
   'src/content/test/data/layout_tests/LayoutTests/storage/domstorage':
      Var("webkit_trunk")[:-6] + '/trunk/LayoutTests/storage/domstorage@119581',
   'src/third_party/snappy/src':
      'http://snappy.googlecode.com/svn/trunk@37',
   'src/content/test/data/layout_tests/LayoutTests/platform/chromium/fast/workers':
      Var("webkit_trunk")[:-6] + '/trunk/LayoutTests/platform/chromium/fast/workers@119581',
   'src/tools/deps2git':
      '/trunk/tools/deps2git@139377',
   'src/third_party/libjpeg_turbo':
      '/trunk/deps/third_party/libjpeg_turbo@139754',
   'src/third_party/v8-i18n':
      'http://v8-i18n.googlecode.com/svn/trunk@104',
   'src/content/test/data/layout_tests/LayoutTests/platform/chromium/fast/events':
      Var("webkit_trunk")[:-6] + '/trunk/LayoutTests/platform/chromium/fast/events@119581',
   'depot_tools':
      '/trunk/tools/depot_tools@140808',
   'src/third_party/bidichecker':
      'http://bidichecker.googlecode.com/svn/trunk/lib@4',
   'src/content/test/data/layout_tests/LayoutTests/platform/chromium-win/http/tests/workers':
      Var("webkit_trunk")[:-6] + '/trunk/LayoutTests/platform/chromium-win/http/tests/workers@119581',
   'src/breakpad/src':
      'http://google-breakpad.googlecode.com/svn/trunk/src@939',
   'src/third_party/pylib':
      'http://src.chromium.org/native_client/trunk/src/third_party/pylib@8673',
   'src/content/test/data/layout_tests/LayoutTests/http/tests/resources':
      Var("webkit_trunk")[:-6] + '/trunk/LayoutTests/http/tests/resources@119581',
   'src/third_party/jsoncpp/source/src/lib_json':
      'http://jsoncpp.svn.sourceforge.net/svnroot/jsoncpp/trunk/jsoncpp/src/lib_json@248',
   'src/content/test/data/layout_tests/LayoutTests/platform/chromium-win/fast/workers':
      Var("webkit_trunk")[:-6] + '/trunk/LayoutTests/platform/chromium-win/fast/workers@119581',
   'build/scripts/command_wrapper/bin':
      '/trunk/tools/command_wrapper/bin@135178',
   'src/testing/gtest':
      'http://googletest.googlecode.com/svn/trunk@613',
   'src/chrome/browser/resources/software_rendering_list':
      '/trunk/deps/gpu/software_rendering_list@140688',
   'src/testing/gmock':
      'http://googlemock.googlecode.com/svn/trunk@405',
   'src/third_party/skia/src':
      'http://skia.googlecode.com/svn/trunk/src@4156',
   'src/chrome/test/data/extensions/api_test/permissions/nacl_enabled/bin':
      'http://src.chromium.org/native_client/trunk/src/native_client/tests/prebuilt@8806',
   'src/third_party/smhasher/src':
      'http://smhasher.googlecode.com/svn/trunk@136',
   'src/third_party/webrtc':
      'http://webrtc.googlecode.com/svn/stable/src@2356',
   'src/native_client':
      'http://src.chromium.org/native_client/trunk/src/native_client@8806',
   'src/tools/page_cycler/acid3':
      '/trunk/deps/page_cycler/acid3@102714',
   'src/third_party/leveldatabase/src':
      'http://leveldb.googlecode.com/svn/trunk@64',
   'src/third_party/mozc/session':
      'http://mozc.googlecode.com/svn/trunk/src/session@83',
   'build':
      '/trunk/tools/build@140762',
   'src/tools/gyp':
      'http://gyp.googlecode.com/svn/trunk@1409',
   'src/chrome/test/data/perf/canvas_bench':
      '/trunk/deps/canvas_bench@122605',
   'src/third_party/ffmpeg':
      '/trunk/deps/third_party/ffmpeg@140400',
   'src/third_party/cacheinvalidation/files/src/google':
      'http://google-cache-invalidation-api.googlecode.com/svn/trunk/src/google@213',
   'src/content/test/data/layout_tests/LayoutTests/platform/chromium-win/storage/domstorage':
      Var("webkit_trunk")[:-6] + '/trunk/LayoutTests/platform/chromium-win/storage/domstorage@119581',
   'src/native_client_sdk/src/site_scons':
      'http://src.chromium.org/native_client/trunk/src/native_client/site_scons@8806',
   'src/content/test/data/layout_tests/LayoutTests/http/tests/xmlhttprequest':
      Var("webkit_trunk")[:-6] + '/trunk/LayoutTests/http/tests/xmlhttprequest@119581',
   'src/third_party/jsoncpp/source/include':
      'http://jsoncpp.svn.sourceforge.net/svnroot/jsoncpp/trunk/jsoncpp/include@248',
   'src/third_party/libphonenumber/src/test':
      'http://libphonenumber.googlecode.com/svn/trunk/cpp/test@456',
   'src/third_party/libsrtp':
      '/trunk/deps/third_party/libsrtp@123853',
   'src/third_party/WebKit/Tools/Scripts':
      Var("webkit_trunk")[:-6] + '/trunk/Tools/Scripts@119581',
   'src/content/test/data/layout_tests/LayoutTests/platform/chromium-win/fast/events':
      Var("webkit_trunk")[:-6] + '/trunk/LayoutTests/platform/chromium-win/fast/events@119581',
   'src/sdch/open-vcdiff':
      'http://open-vcdiff.googlecode.com/svn/trunk@42',
   'src/third_party/pymox/src':
      'http://pymox.googlecode.com/svn/trunk@70',
   'src/third_party/libjingle/source':
      'http://libjingle.googlecode.com/svn/trunk@151',
   'src/content/test/data/layout_tests/LayoutTests/fast/events':
      Var("webkit_trunk")[:-6] + '/trunk/LayoutTests/fast/events@119581',
   'src/third_party/WebKit':
      '/trunk/deps/third_party/WebKit@76115',
   'src/third_party/icu':
      '/trunk/deps/third_party/icu46@140144',
   'src/third_party/speex':
      '/trunk/deps/third_party/speex@111570',
   'src/tools/grit':
      'http://grit-i18n.googlecode.com/svn/trunk@45',
   'src/third_party/WebKit/Tools/DumpRenderTree':
      Var("webkit_trunk")[:-6] + '/trunk/Tools/DumpRenderTree@119581',
   'src/third_party/webpagereplay':
      'http://web-page-replay.googlecode.com/svn/trunk@468',
   'src/seccompsandbox':
      'http://seccompsandbox.googlecode.com/svn/trunk@186',
   'src/third_party/yasm/source/patched-yasm':
      '/trunk/deps/third_party/yasm/patched-yasm@134927',
   'src/third_party/swig/Lib':
      '/trunk/deps/third_party/swig/Lib@69281',
   'src/third_party/WebKit/Tools/TestWebKitAPI':
      Var("webkit_trunk")[:-6] + '/trunk/Tools/TestWebKitAPI@119581',
   'src/third_party/angle':
      'http://angleproject.googlecode.com/svn/trunk@1046',
   'src/v8':
      'http://v8.googlecode.com/svn/trunk@11689',
   'src/third_party/libvpx':
      '/trunk/deps/third_party/libvpx@134182',
   'src/content/test/data/layout_tests/LayoutTests/http/tests/websocket/tests':
      Var("webkit_trunk")[:-6] + '/trunk/LayoutTests/http/tests/websocket/tests@119581',
   'src/content/test/data/layout_tests/LayoutTests/fast/js/resources':
      Var("webkit_trunk")[:-6] + '/trunk/LayoutTests/fast/js/resources@119581',
}

skip_child_includes =  ['breakpad', 'chrome_frame', 'delegate_execute', 'metro_driver', 'native_client', 'native_client_sdk', 'o3d', 'pdf', 'sdch', 'skia', 'testing', 'third_party', 'v8'] 

hooks =  [{'action': ['python', 'src/build/download_nacl_toolchains.py', '--no-arm-trusted', '--optional-pnacl', '--pnacl-version', '8751', '--file-hash', 'pnacl_linux_x86_32', '36d1121c16b249fe11c24b3f149a75a9da906517', '--file-hash', 'pnacl_linux_x86_64', '01aed2db78ee87cccf7599931c87fea244e8bd54', '--file-hash', 'pnacl_translator', '6d8c5b8c0613556f39e5132d1a7b9b65e020e01e', '--file-hash', 'pnacl_mac_x86_32', '333538f719ddd7742cda120cc1b70ced4d1d72ed', '--file-hash', 'pnacl_win_x86_32', '32c786bb095acfa0a0c5942e355af2ef03dec9dd', '--x86-version', '8715', '--file-hash', 'mac_x86_newlib', '4f072a30b55ce9088ed6c6a0033dffa3a8ae301b', '--file-hash', 'win_x86_newlib', 'cd4f8209a6f87d8baf21d1610928a3def4306764', '--file-hash', 'linux_x86_newlib', '849a8cf371e17a685b9429a69474f328454b5e06', '--file-hash', 'mac_x86', '42575a21646027d52778240c03246d4534c98956', '--file-hash', 'win_x86', '85d9b16cb503a10963654803c4a4f90ea6edfff2', '--file-hash', 'linux_x86', 'd82ef6eea6ff6ba427efa70461c2fc411d64569f', '--save-downloads-dir', 'src/native_client_sdk/src/build_tools/toolchain_archives', '--keep'], 'pattern': '.'}, {'action': ['python', 'src/tools/clang/scripts/update.py', '--mac-only'], 'pattern': '.'}, {'action': ['python', 'src/build/win/setup_cygwin_mount.py', '--win-only'], 'pattern': '.'}, {'action': ['python', 'src/build/util/lastchange.py', '-o', 'src/build/util/LASTCHANGE'], 'pattern': '.'}, {'action': ['python', 'src/build/gyp_chromium'], 'pattern': '.'}] 

include_rules =  ['+base', '+build', '+googleurl', '+ipc', '+unicode', '+testing']