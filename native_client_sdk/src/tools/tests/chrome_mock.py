#!/usr/bin/env python
# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import optparse
import sys
import time
import urllib2

def PrintAndFlush(s):
  print s
  sys.stdout.flush()

def main(args):
  parser = optparse.OptionParser(usage='%prog [options] <URL to load>')
  parser.add_option('--post', help='POST to URL.', dest='post',
                    action='store_true')
  parser.add_option('--get', help='GET to URL.', dest='get',
                    action='store_true')
  parser.add_option('--sleep',
                    help='Number of seconds to sleep after reading URL',
                    dest='sleep', default=0)
  parser.add_option('--expect-to-be-killed', help='If set, the script will warn'
                    ' if it isn\'t killed before it finishes sleeping.',
                    dest='expect_to_be_killed', action='store_true')
  options, args = parser.parse_args(args)
  if len(args) != 1:
    parser.error('Expected URL to load.')

  PrintAndFlush('Starting %s.' % sys.argv[0])

  if options.post:
    urllib2.urlopen(args[0], data='').read()
  elif options.get:
    urllib2.urlopen(args[0]).read()
  else:
    # Do nothing but wait to be killed.
    pass

  time.sleep(float(options.sleep))

  if options.expect_to_be_killed:
    PrintAndFlush('Done sleeping. Expected to be killed.')
  return 0

if __name__ == '__main__':
  sys.exit(main(sys.argv[1:]))
