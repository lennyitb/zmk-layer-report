#!/usr/bin/env bash
set -euo pipefail

GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
BOLD='\033[1m'
NC='\033[0m'

WEST_YML="config/west.yml"
REMOTE_NAME="lennyitb"
MODULE_NAME="zmk-layer-report"
CONFIG_LINE="CONFIG_ZMK_LAYER_REPORT=y"

info()  { printf "${GREEN}✓${NC} %s\n" "$1"; }
warn()  { printf "${YELLOW}•${NC} %s\n" "$1"; }
fail()  { printf "${RED}✗${NC} %s\n" "$1" >&2; exit 1; }

prompt_input() {
    local prompt_msg="$1"
    printf "%s" "$prompt_msg"
    if [[ -t 0 ]]; then
        read -r REPLY
    elif [[ -c /dev/tty ]]; then
        read -r REPLY </dev/tty
    else
        read -r REPLY
    fi
}

# --- Validate ---

if [[ ! -f "$WEST_YML" ]]; then
    fail "config/west.yml not found — run this from the root of your ZMK config repo."
fi

echo ""
echo -e "${BOLD}Installing $MODULE_NAME${NC}"
echo ""

# --- Update west.yml ---

if grep -q "$MODULE_NAME" "$WEST_YML"; then
    warn "$MODULE_NAME already in $WEST_YML — skipping"
else
    needs_remote=true
    grep -q "name: $REMOTE_NAME" "$WEST_YML" && needs_remote=false

    awk -v add_remote="$needs_remote" '
    /^  projects:/ {
        if (add_remote == "true") {
            print "    - name: lennyitb"
            print "      url-base: https://github.com/lennyitb"
        }
        print
        next
    }
    /^  self:/ {
        print "    - name: zmk-layer-report"
        print "      remote: lennyitb"
        print "      revision: main"
        print
        next
    }
    { print }
    ' "$WEST_YML" > "$WEST_YML.tmp"

    if ! grep -q "$MODULE_NAME" "$WEST_YML.tmp"; then
        rm -f "$WEST_YML.tmp"
        fail "Failed to update $WEST_YML — your file may have non-standard formatting.\n  See the README for manual instructions."
    fi

    mv "$WEST_YML.tmp" "$WEST_YML"
    info "Updated $WEST_YML"
fi

# --- Update .conf ---

conf_files=()
while IFS= read -r f; do
    [[ -n "$f" ]] && conf_files+=("$f")
done < <(find config -maxdepth 1 -name '*.conf' 2>/dev/null | sort)

if [[ ${#conf_files[@]} -eq 0 ]]; then
    echo ""
    warn "No .conf files found in config/"
    echo "  Add this line to your board's .conf file:"
    echo "    $CONFIG_LINE"
else
    # Filter to files that don't already have the line
    need_conf=()
    already_conf=()
    for f in "${conf_files[@]}"; do
        if grep -q "^${CONFIG_LINE}$" "$f"; then
            already_conf+=("$f")
        else
            need_conf+=("$f")
        fi
    done

    if [[ ${#already_conf[@]} -gt 0 ]]; then
        for f in "${already_conf[@]}"; do
            warn "$CONFIG_LINE already in $f — skipping"
        done
    fi

    if [[ ${#need_conf[@]} -eq 1 ]]; then
        echo "$CONFIG_LINE" >> "${need_conf[0]}"
        info "Added $CONFIG_LINE to ${need_conf[0]}"
    elif [[ ${#need_conf[@]} -gt 1 ]]; then
        echo ""
        echo "Which .conf file is your main keyboard half?"
        for i in "${!need_conf[@]}"; do
            echo "  $((i + 1))) ${need_conf[$i]}"
        done
        echo ""
        prompt_input "Enter number [1-${#need_conf[@]}]: "

        if [[ "$REPLY" =~ ^[0-9]+$ ]] && (( REPLY >= 1 && REPLY <= ${#need_conf[@]} )); then
            selected="${need_conf[$((REPLY - 1))]}"
            echo "$CONFIG_LINE" >> "$selected"
            info "Added $CONFIG_LINE to $selected"
        else
            echo ""
            warn "No file selected. Add this line to your main half's .conf manually:"
            echo "    $CONFIG_LINE"
        fi
    fi
fi

echo ""
info "Done — build your firmware as usual and west will pull in the module."
echo ""
