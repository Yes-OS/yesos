#!/bin/bash

git filter-branch --env-filter '

n=$GIT_AUTHOR_NAME
m=$GIT_AUTHOR_EMAIL

case ${GIT_AUTHOR_NAME} in
  ajmadse2) n="Adam Madsen" ; m="madsen.adam@gmail.com" ;;
  hskllgl2) n="Ryan Haskell-Glatz" ; m="hskllgl2@illinois.edu" ;;
  taberna2) n="Artur Tabernacki" ; m="atabernacki@gmail.com" ;;
  vinjamu2) n="Raj Vinjamuri" ; m="rajvinjamuri@gmail.com" ;;
esac

export GIT_AUTHOR_NAME="$n"
export GIT_AUTHOR_EMAIL="$m"
export GIT_COMMITTER_NAME="$n"
export GIT_COMMITTER_EMAIL="$m"
'
