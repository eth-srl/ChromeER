// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TOOLS_GN_ACTION_VALUES_H_
#define TOOLS_GN_ACTION_VALUES_H_

#include <string>
#include <vector>

#include "base/basictypes.h"
#include "tools/gn/source_file.h"
#include "tools/gn/substitution_list.h"

// Holds the values (outputs, args, script name, etc.) for either an action or
// an action_foreach target.
class ActionValues {
 public:
  ActionValues();
  ~ActionValues();

  // Filename of the script to execute.
  const SourceFile& script() const { return script_; }
  void set_script(const SourceFile& s) { script_ = s; }

  // Arguments to the script.
  SubstitutionList& args() { return args_; }
  const SubstitutionList& args() const { return args_; }

  // Files created by the script. These are strings rather than SourceFiles
  // since they will often contain {{source expansions}}.
  SubstitutionList& outputs() { return outputs_; }
  const SubstitutionList& outputs() const { return outputs_; }

  // Depfile generated by the script.
  const SubstitutionPattern& depfile() const { return depfile_; }
  bool has_depfile() const { return !depfile_.ranges().empty(); }
  void set_depfile(const SubstitutionPattern& depfile) { depfile_ = depfile; }

 private:
  SourceFile script_;
  SubstitutionList args_;
  SubstitutionList outputs_;
  SubstitutionPattern depfile_;

  DISALLOW_COPY_AND_ASSIGN(ActionValues);
};

#endif  // TOOLS_GN_ACTION_VALUES_H_
