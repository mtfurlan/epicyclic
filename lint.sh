#!/usr/bin/env bash
set -euo pipefail
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd "$DIR" &>/dev/null

# shellcheck disable=SC2120
h () {
    # if arguments, print them
    [ $# == 0 ] || echo "$*"

  cat <<EOF
Usage: $(basename "${BASH_SOURCE[0]}") [OPTION...]
Do a bunch of static analysis and documentation checks on all some files
File selection logic:
* If any files are staged, it will only run on those
* If any files are unstaged, it will only run on those
* If the working directory is clean, it will run on all files
It will add format and typo changes to the git index if anything is staged
* ensure clang-format is happy, prompt with fixes if in interactive mode
* ensure no merge markers
* ensure no TODOs, by default just warn about them
* ensure README.md table of contents updated, prompt if in interactive mode
Available options:
  -h, --help             display this help and exit
      --non-interactive  just fail if the linter is unhappy,don't prompt
      --todo-fail        fail on todos, don't just warn
  -a, --lint-all         lint everything, not just changed files
  -y, --yes              accept all linter changes
Return Codes:
  0: successfully passed lint
  2: failed to pass lint
  *: unknown failure
EOF

    # if args, exit 1 else exit 0
    [ $# == 0 ] || exit 1
    exit 0
}

colorMessage() {
    color=$1
    shift
    echo "${color}$*${NOFORMAT}"
}
redmsg() {
    colorMessage "${RED}" "$*${NOFORMAT}"
}
yellowmsg() {
    colorMessage "${YELLOW}" "$*${NOFORMAT}"
}
die() {
    local msg=$1
    local code=${2-1} # default exit status 1
    echo "$msg"
    exit "$code"
}

# https://stackoverflow.com/a/29613573/2423187
# SYNOPSIS
#   quoteRe <text>
# shellcheck disable=SC1003
quoteRe() { sed -e 's/[^^]/[&]/g; s/\^/\\^/g; $!a\'$'\n''\\n' <<<"$1" | tr -d '\n'; }

# SYNOPSIS
#  quoteSubst <text>
quoteSubst() {
  IFS= read -d '' -r < <(sed -e ':a' -e '$!{N;ba' -e '}' -e 's/[&/\]/\\&/g; s/\n/\\&/g' <<<"$1")
  printf %s "${REPLY%$'\n'}"
}


#shellcheck disable=SC2034
setup_colors() {
  if [[ -t 2 ]] && [[ -z "${NO_COLOR-}" ]] && [[ "${TERM-}" != "dumb" ]] && command -v tput >/dev/null 2>&1 ; then
    NOFORMAT="$(tput sgr0)" RED="$(tput setaf 1)" GREEN="$(tput setaf 2)" YELLOW="$(tput setaf 3)" BLUE="$(tput setaf 4)" MAGENTA="$(tput setaf 5)" CYAN="$(tput setaf 6)" WHITE="$(tput setaf 7)"
  else
    NOFORMAT='' RED='' GREEN='' YELLOW='' BLUE='' MAGENTA='' CYAN='' WHITE=''
  fi
}


lint() {
    # if not lint all and there are staged changes
    if [ "$lintAll" == false ] && ! git diff --cached --quiet; then
        staged=true
        stagedArg=(--index -p0)
    else
        staged=false
        stagedArg=()
    fi

    # figure out what files to lint by listing all files git knows about and
    # removing deleted and stuff in tooling/lintExcludes

    fileExcludes=$(cat "$DIR/lintExcludes" 2>/dev/null || echo -e "BSD_grep_needs_something"; git ls-files -d)

    # `git grep --cached -Il ''` will list all non-binary files (git ls-files includes submodules)
    # grep -v finds everything not in -f PATTERN_FILE
    filenames=$(grep -v -f <(echo "$fileExcludes") <(git grep --cached -Il ''))

    #if staged, remove non-staged files from list to check
    if [ "$staged" == true ]; then
        ret=1
        #shellcheck disable=SC2001
        filenames=$(grep -f <(echo "$filenames" | sed 's/\(.*\)/^\1$/') <(git diff --cached --name-only)) || ret=$?
        if [ "$ret" -ne 1 ]; then
            die "grep error"
        fi
    # if not lint all only lint changed
    elif [ "$lintAll" == false ]; then
        ret=1
        #shellcheck disable=SC2001
        filenames=$(grep -f <(echo "$filenames" | sed 's/\(.*\)/^\1$/') <(git diff --name-only HEAD)) || ret=$?
        if [ "$ret" -ne 1 ]; then
            die "grep error"
        fi
    fi


    # if filenames only contain whitespace, we lint nothing because it was just
    # deletions or something
    if ! [[ "$filenames" =~ [^[[:space:]] ]]; then
        exit 0
    fi

    mapfile -t filenamesArr < <(echo "$filenames")

    ########################### MERGE MARKERS #################################
    # git merge uses 7 <, >, or |, followed by a specification of some kind
    # it also uses 7 =, not followed by anything
    # unfortunately, 7 = is also used in the wild
    diffMarkers=$(grep -E '^[>|<]{7} ' -H -I "${filenamesArr[@]}" || true)

    ## If the egrep command has any hits - echo a warning and exit with non-zero status.
    if [[ -n "$diffMarkers" ]]; then
        echo "$diffMarkers"
        die "\n${RED}WARNING: You have merge markers in the above files, lines. Fix them before committing.${NOFORMAT}" 2
    fi


    fail=false
    ignored=0

    ########################## TRAILING WHITESPACE ############################
    for file in "${filenamesArr[@]}"; do
        if grep -q "\s$" "$file"; then
            echo "Trailing whitespace in $file. Fix your editor."
            if [ "$interactive" == true ] || [ "$acceptAllLinter" == true ]; then
                if [ "$acceptAllLinter" == true ]; then
                    lint_response=y
                else
                    read -r -e -p "Remove? [y/N] " lint_response
                fi
                case "$lint_response" in
                    [yY][eE][sS]|[yY])
                        ret=0
                        sed -i 's/[ \t]*$//' "$file"
                        ;;
                    *)
                        echo "You have chosen to keep your file, ill-formatted as it is...good luck."
                        ((ignored += 1))
                        ;;
                esac
            else
                fail=true
            fi
        fi
        if test "$(tail -c1 "$file")"; then
            echo "No newline at end of $file. Fix your editor, in vscode it's called 'insert final newline'."
            if [ "$interactive" == true ] || [ "$acceptAllLinter" == true ]; then
                if [ "$acceptAllLinter" == true ]; then
                    lint_response=y
                else
                    read -r -e -p "Fix? [y/N] " lint_response
                fi
                case "$lint_response" in
                    [yY][eE][sS]|[yY])
                        ret=0
                        #shellcheck disable=SC1003
                        sed -i -e '$a\' "$file"
                        ;;
                    *)
                        echo "You have chosen to keep your file, ill-formatted as it is...good luck."
                        ((ignored += 1))
                        ;;
                esac
            else
                fail=true
            fi
        fi
    done

    ########################### TYPOS #########################################
    #if ! command -v typos >/dev/null 2>&1; then
    #    yellowmsg "typos not installed, not testing. Please commit with --no-verify if you don't want to install"
    #    fail=true
    #else
    #    ret=0
    #    # display typos in less if interactive, else just print
    #    if [ "$interactive" == true ] && [ "$acceptAllLinter" != true ]; then
    #        echo "$filenames" | typos --config tooling/typos.toml --file-list - --color always | less -RFX || ret=$?
    #    else
    #        echo "$filenames" | typos --config tooling/typos.toml --file-list - || ret=$?
    #    fi
    #    if [ "$ret" -ne 0 ]; then
    #        if [ "$interactive" == true ] || [ "$acceptAllLinter" == true ]; then
    #            if [ "$acceptAllLinter" == true ]; then
    #                lint_response=y
    #            else
    #                read -r -e -p "Apply typos changes? [y/N] " lint_response
    #            fi
    #            case "$lint_response" in
    #                [yY][eE][sS]|[yY])
    #                    ret=0
    #                    echo "$filenames" | typos --config tooling/typos.toml  --file-list - --diff | git apply --allow-empty -v --unidiff-zero -p0 "${stagedArg[@]}"  || ret=$?
    #                    if [  "$ret" -ne 0 ]; then
    #                        echo "an error occurred when fixing typos"
    #                        echo "probably there was more than once choice so you have to manually fix it"
    #                        fail=true
    #                    fi
    #                    ;;
    #                *)
    #                    echo "You have chosen to keep your file, ill-formatted as it is...good luck.\n"
    #                    ((ignored += 1))
    #                    ;;
    #            esac
    #        else
    #            echo "typos unhappy"
    #            fail=true
    #        fi
    #    fi
    #fi

    ########################## CLANG-FORMAT ###################################
    if ! command -v clang-format >/dev/null 2>&1; then
        yellowmsg "clang-format not installed, not testing. Please commit with --no-verify if you don't want to install"
        fail=true
    else
        lintedFiles=()
        trap '{ rm -f "${lintedFiles[@]}"; }' EXIT
        mapfile -t cishfiles < <(printf "%s\n" "${filenamesArr[@]}" | grep -E '(\.h$|\.c$|\.cpp$|\.hpp$)' || true)
        for file in "${cishfiles[@]}"; do
            patch="$file.patch"
            rm -f "$patch"

            if [ "$staged" == true ]; then
                (git diff -U0 --no-color --staged "$file" | clang-format-diff -style file:.clang-format -p1 || true) > "$patch"
            else
                lintedFile=$(mktemp)
                lintedFiles+=("$lintedFile")
                clang-format --style=file:.clang-format "$file" > "$lintedFile"
                (git diff --no-index "$file" "$lintedFile" || true) > "$patch"
                sed -i "s/$(quoteRe "$lintedFile")/\/$(quoteSubst "$file")/" "$patch"
            fi

            if [ ! -s "$patch" ];  then
                # empty file is fine
                rm "$patch"
                continue
            fi

            #else diff

            # if we aren't interactive, mark it as failed and keep going
            if [ "$interactive" != true ]; then
                echo "clang-format was unhappy with $file"
                ((ignored += 1))
                continue
            fi

            if [ "$acceptAllLinter" == true ]; then
                lint_response=y
            else
                #There is a diff
                # get git diff viewer
                diffViewer=$(git config --get interactive.diffFilter || true)
                if [ -z "$diffViewer" ]; then
                    if command -v colordiff >/dev/null 2>&1; then
                        diffViewer="colordiff"
                    else
                        diffViewer="cat"
                    fi
                fi
                if [ "$interactive" == true ]; then
                    $diffViewer < "$patch" | less -RFX
                else
                    $diffViewer < "$patch"
                fi

                read -r -e -p "Apply clang-format changes? [y/N] " lint_response
            fi
            case "$lint_response" in
                [yY][eE][sS]|[yY])
                    ret=0
                    git apply "${stagedArg[@]}" < "$patch" || ret=$?
                    if [  "$ret" -ne 0 ]; then
                        echo "an error occurred when applying clang-format patch $patch: $ret"
                        fail=true
                    else
                        rm "$patch"
                    fi
                    ;;
                *)
                    echo -e "You have chosen to keep your file, ill-formatted as it is...good luck.\nLinter suggestion is in $patch\n"
                    ((ignored += 1))
                    ;;
            esac
        done

        # fail if user didn't apply any changes or not interactive
        if [ "$ignored" -gt 0 ]; then
            redmsg "You didn't accept all the changes, or this wasn't run interactively. Good luck either way."
            clang-format --version
            fail=true
        fi
    fi


    ################################### TODO ###################################
    # ignore this file when searching for TODO
    # It's easier.
    mapfile -t todoFiles < <(printf "%s\n" "${filenamesArr[@]}" | grep -v "$(basename "${BASH_SOURCE[0]}")$" || true)
    if [[ "${#todoFiles[@]}" -gt 0 ]]; then
        todoMarkers=$(grep -i 'TODO' -H -I "${todoFiles[@]}" || true)

        if [[ -n "$todoMarkers" ]]; then
            todoColour=""
            if [ "$todoFail" == true ]; then
                todoColour=${RED}
            else
                todoColour=${YELLOW}
            fi
            colorMessage "$todoColour" "WARNING: You have TODOs. Please fix them or make up a policy for this project"
            echo -e "\n$todoMarkers\n"

            if [ "$todoFail" == true ]; then
                fail=true
            fi
        fi
    fi

    if [ "$fail" == true ]; then
        die "linty disapproves" 2
    fi

}

# check getopt version
ret=0
getopt -T > /dev/null || ret=$?
if [ ! $ret -eq 4 ]; then
    die "You have the wrong getopt. We use fancy long arguments."
fi



# getopt short options go together, long options have commas
TEMP=$(getopt -o hay --long help,all,non-interactive,todo-fail,yes,lint-all -n "$0" -- "$@")
#shellcheck disable=SC2181
if [ $? != 0 ] ; then
    die "something wrong with getopt"
fi
eval set -- "$TEMP"

if git diff --quiet HEAD; then
    lintAll=true
else
    lintAll=false
fi
todoFail=false
acceptAllLinter=false
interactive=true
while true ; do
    case "$1" in
        -h|--help) h ;;
        --non-interactive) interactive=false ; shift ;;
        --todo-fail) todoFail=true ; shift ;;
        -a|--lint-all) lintAll=true ; shift ;;
        -y|--yes) acceptAllLinter=true ; shift ;;
        --) shift ; break ;;
        *) die "issue parsing args, unexpected argument '$0'!" ;;
    esac
done

# if we are interactive but no stdin, do stuff
if [ "$interactive" == true ] && [ ! -t "0" ]; then
    exec < /dev/tty
fi

setup_colors

lint
