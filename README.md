# Phosh OSK Stub

A tool to debug input related issues in phosh. For a real on screen keyboard
see [squeekboard][].

## License

phosh-osk-stub is licensed under the GPLv3+.

## Getting the source

```sh
git clone https://gitlab.gnome.org/guidog/phosh-osk-stub
cd phosh-osk-stub
```

The [main][] branch has the current development version.

## Dependencies
On a Debian based system run

```sh
sudo apt-get -y install build-essential
sudo apt-get -y build-dep .
```

For an explicit list of dependencies check the `Build-Depends` entry in the
[debian/control][] file.

## Building

We use the meson (and thereby Ninja) build system for phosh.  The quickest
way to get going is to do the following:

```sh
meson . _build
ninja -C _build
```

## Running
### Running from the source tree
When running from the source tree first start *[phosh][]*.
Then start *phosh-osk-stub* using:

```sh
_build/src/phosh-osk-stub
```

Note that there's no need to install any files outside the source tree.

The result should look something like this:

![debug surface](screenshots/pos-dbg.png)
![character popover](screenshots/pos-de.png)
![inscript/malayalam](screenshots/pos-wide-in+mal.png)

[main]: https://gitlab.gnome.org/guidog/phosh-osk-stub/-/tree/main
[.gitlab-ci.yml]: https://gitlab.gnome.org/guidog/phosh-osk-stub/-/blob/main/.gitlab-ci.yml
[debian/control]:https://gitlab.gnome.org/guidog/phosh-osk-stub/-/blob/main/debian/control
[phosh]: https://gitlab.gnome.org/World/Phosh/phosh
[squeekboard]: https://gitlab.gnome.org/World/Phosh/squeekboard
