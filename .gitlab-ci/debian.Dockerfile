FROM debian:trixie-slim

RUN export DEBIAN_FRONTEND=noninteractive \
   && apt-get -y update \
   && apt-get -y install --no-install-recommends wget ca-certificates gnupg eatmydata \
   && eatmydata apt-get -y update \
   && eatmydata apt-get -y upgrade \
   && cd /home/user/app \
   && eatmydata apt-get --no-install-recommends -y build-dep . \
   && eatmydata apt-get --no-install-recommends -y install build-essential git wget gcovr locales uncrustify \
   && eatmydata apt-get clean

# Work around broken gi-docgen 2023.1+ds-4 in trixie:
RUN export DEBIAN_FRONTEND=noninteractive \
   && echo "deb http://deb.debian.org/debian/ sid main" > /etc/apt/sources.list.d/sid.list \
   && eatmydata apt-get update \
   && eatmydata apt-get install gi-docgen \
   && rm -f /etc/apt/sources.list.d/sid \
   && eatmydata apt-get clean
