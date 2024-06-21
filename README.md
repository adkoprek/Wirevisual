# Wirevisual

---

Generic qt scan appilication for HIPA

Author: Adam Koprek adam.koprek@psi.ch

## Dependencies

- Qt5
- Epics Libraries
- Cafe: [GitLab Repo](https://git.psi.ch/cafe)

## Build

tested on hipa-lc8

1. Clone the repository
  
  ```shell
  git clone git@git.psi.ch:hipa_apps/Wirevisual
  ```
  
2. Create a build area
  
  ```shell
  mkdir cmake
  cd cmake
  ```
  
3. Configure and make
  
  ```shell
  cmake .. -DCMAKE_BUILD_TYPE=Release
  make
  ```
  
4. Run the app
  
  ```shell
  ../bin/<Your Procesor Architecture>/Release/wirevisual
  ```
