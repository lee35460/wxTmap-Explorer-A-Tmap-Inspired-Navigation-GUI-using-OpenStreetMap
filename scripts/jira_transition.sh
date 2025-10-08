#!/usr/bin/env bash
set -euo pipefail

# Required envs must be provided via GitHub Secrets or runner env
: "${JIRA_BASE_URL:?JIRA_BASE_URL is required}"
: "${JIRA_USER_EMAIL:?JIRA_USER_EMAIL is required}"
: "${JIRA_API_TOKEN:?JIRA_API_TOKEN is required}"

jira_get_transition_id() {
  local key="$1" name="$2"
  curl -sS -u "$JIRA_USER_EMAIL:$JIRA_API_TOKEN" \
    -H "Accept: application/json" \
    "$JIRA_BASE_URL/rest/api/3/issue/$key/transitions" \
  | jq -r --arg name "$name" '.transitions[] | select(.name==$name) | .id' || true
}

jira_transition() {
  local key="$1" name="$2"
  local tid
  tid="$(jira_get_transition_id "$key" "$name")"
  if [[ -z "${tid:-}" || "$tid" == "null" ]]; then
    echo "WARN: No transition named '$name' for $key (skipping)"
    return 0
  fi

  curl -sS -X POST \
    -u "$JIRA_USER_EMAIL:$JIRA_API_TOKEN" \
    -H "Content-Type: application/json" \
    --data "{\"transition\": {\"id\": \"$tid\"}}" \
    "$JIRA_BASE_URL/rest/api/3/issue/$key/transitions" \
    >/dev/null

  echo "OK: $key -> $name (id=$tid)"
}

# Extract issue keys (WXT-123), case-insensitive
extract_keys() {
  local text="${1:-}"
  [[ -z "$text" ]] && return 0
  echo "$text" | grep -Eoi 'WXT-[0-9]+' | awk '{print toupper($0)}' | sort -u
}

run_for_keys() {
  local keys_csv="${1:-}" to="${2:-}"
  if [[ -z "$keys_csv" || -z "$to" ]]; then
    echo "ERR: keys or transition name missing" >&2
    exit 1
  fi
  IFS=',' read -ra KEYS <<< "$keys_csv"
  for k in "${KEYS[@]}"; do
    [[ -z "$k" ]] && continue
    jira_transition "$k" "$to"
  done
}

case "${1:-}" in
  find-from-branch)
    extract_keys "${2:-}"
    ;;
  find-from-text)
    extract_keys "${2:-}"
    ;;
  do)
    run_for_keys "${2:-}" "${3:-}"
    ;;
  *)
    echo "Usage:"
    echo "  $0 find-from-branch <branch_name>"
    echo "  $0 find-from-text <text>"
    echo "  $0 do <KEY1,KEY2,...> <Transition Name>"
    exit 1
    ;;
esac