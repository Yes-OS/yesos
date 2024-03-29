#!/bin/bash

# borrowed from http://stackoverflow.com/questions/392332/retroactively-correct-authors-with-git-svn
# Rewrites git history to change authors' emails to correct versions
# Dangerous. Do not use unless you know what you're doing.

git filter-branch --env-filter '

n=$GIT_AUTHOR_NAME
m=$GIT_AUTHOR_EMAIL

case ${GIT_AUTHOR_NAME} in
  ajmadse2|"Adam Madsen") n="Adam Madsen" ; m="madsen.adam@gmail.com" ;;
  hskllgl2|"Ryan Haskell-Glatz") n="Ryan Haskell-Glatz" ; m="hskllgl2@illinois.edu" ;;
  taberna2|"Artur Tabernacki") n="Artur Tabernacki" ; m="atabernacki@gmail.com" ;;
  vinjamu2|"Raj Vinjamuri") n="Raj Vinjamuri" ; m="rajvinjamuri@gmail.com" ;;
esac

export GIT_AUTHOR_NAME="$n"
export GIT_AUTHOR_EMAIL="$m"
export GIT_COMMITTER_NAME="$n"
export GIT_COMMITTER_EMAIL="$m"
'
