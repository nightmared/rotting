#!/usr/bin/env python3

import re, sys
from string import Template
from enum import Enum

class line_type(Enum):
  open_block = 1
  text = 2
  close_block = 3

attrs = dict()

# Regexpes used for matching
attr_regexp = re.compile("^#\S+(=[\S ]+)?$")
attr_without_value_regexp = re.compile("^#(?P<attr_name>\S+)$")
attr_with_value_regexp = re.compile("^#(?P<attr_name>\S+)=(?P<attr_value>[\S ]+)$")
text_regexp = re.compile("^[\S ]+$")
break_regexp = re.compile("^\n$")

def def_attr(line):
  with_value = attr_with_value_regexp.match(line)
  if with_value:
    attrs[with_value.group('attr_name')] = with_value.group('attr_value')
  else:
    without_value = attr_without_value_regexp.match(line) 
    attrs[with_value.group('attr_name')] = true 

def last_tree(tree):
  if len(tree) > 0:
    return file_tree[len(tree)-1]
  else:
    return None

def match_line(file_tree, line):
  if attr_regexp.match(line):
    def_attr(line)

  elif text_regexp.match(line):
    # open block if not already open
    if not last_tree(file_tree) or last_tree(file_tree)[0] == line_type.close_block:
      file_tree.append((line_type.open_block, True))
    file_tree.append((line_type.text, line))

  # close block if '\n' encountered
  elif break_regexp.match(line):
    # Do not close block if not already open
    if last_tree(file_tree) and last_tree(file_tree)[0] == line_type.text:
      file_tree.append((line_type.close_block, True))

# Run latex command with some value
def latex_action(command, value):
  template = Template("\$command{$value}")
  return template.substitute(command=command,value=value)

def generate_document(doc_tree):
  block_no = 0
  document = []
  refrain_size = 0
  # Add a section for the maketitle command
  document.append(latex_action("section", attrs['titre']))
  for line in doc_tree:
    if line[0] == line_type.text:
      document.append(line[1] + "\\\\")

    elif line[0] == line_type.open_block:
      document.append(latex_action("begin", "frame"))
      if block_no == 0:
        document.append(latex_action("frametitle", "{} - Refrain".format(attrs['titre'])))
      else:
        document.append(latex_action("frametitle", "Couplet {}".format(block_no)))

    elif line[0] == line_type.close_block:
      document.append(latex_action("end", "frame"))
      if block_no == 0:
        refrain_size = len(document)
      else:
        document.extend(document[1:refrain_size])
      block_no += 1
  return document


if len(sys.argv) == 2:
  file_tree = []

  for line in open(sys.argv[1],'r').readlines():
     match_line(file_tree, line)
  # close text block if still open
  if last_tree(file_tree)[0] == line_type.text:
    file_tree.append((line_type.close_block, True))
  
  for line in generate_document(file_tree):
    print(line)
