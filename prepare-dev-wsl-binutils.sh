#!/bin/bash
sudo apt install binfmt-support
sudo update-binfmts --install WindowsBat /init --extension bat
sudo update-binfmts --install WindowsCmd /init --extension cmd

