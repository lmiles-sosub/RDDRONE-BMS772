# RDDRONE-BMS772
NuttX source code for RDDRONE-BMS772

This readme files will explain how to get the right nuttx and nuttx-apps repository with the BMS3.4 patches and build the BMS software (create a binary file)
This will work best on a linux machine, you could use a virtual machine for it.

Make a usefull folder to have the files in, like drones or RDDRONE-BMS772
Linux command:
mkdir -p drones
cd drones

Clone the nuttx and nuttx apps git repositories
For nuttx clone:
https://github.com/apache/incubator-nuttx.git
Git command:

git clone https://github.com/apache/incubator-nuttx.git

For nuttx-apps clone:
https://github.com/apache/incubator-nuttx-apps.git
Git command:

git clone https://github.com/apache/incubator-nuttx-apps.git

Make symbolic links for the nuttx and the apps
Linux command:

ln -s incubator-nuttx nuttx

ln -s incubator-nuttx-apps apps

In the apps directory, checkout the right commit and branch
Git hash: 7a85bc75dcf5632154a09e75cfc431b6e25df463
Linux command:

(cd apps; git checkout 7a85bc75dcf5632154a09e75cfc431b6e25df463 -b bms772)

Make a nxp_bms folder in the apps 
Linnux command:

mkdir -p apps/nxp_bms

Make the BMS_v1 folder in the nxp_bms folder
Linnux command:

mkdir -p apps/nxp_bms/BMS_v1

Copy the scr and inc folder in the BMS_v1 folder.
cp -r folder_directory_src apps/nxp_bms/BMS_v1
cp -r folder_directory_inc apps/nxp_bms/BMS_v1
Example:

cp -r src apps/nxp_bms/BMS_v1

cp -r inc apps/nxp_bms/BMS_v1

Checkout the public regulated data types
https://github.com/PX4/public_regulated_data_types
Linux command:

(cd apps/nxp_bms/BMS_v1; git clone https://github.com/PX4/public_regulated_data_types)

Apply the patch to the nuttx-apps
patch -p1 < Patchfiles/0001-apps-patch-BMS3.4.patch
Linux command:

(cd apps; patch -p1 < Patchfiles/0001-apps-patch-BMS3.4.patch)

Go the nuttx folder and checkout the right NuttX commit
Git command:
git checkout 1115f0104bd404454b72e9e02f4583febb0fe162 -b bms772
Linux command:

cd nuttx

git checkout 1115f0104bd404454b72e9e02f4583febb0fe162 -b bms772

Add the nuttx patch 
patch -p1 < Patchfiles/0001-nuttx-patch-BMS3.4.patch
Linux command:

patch -p1 < Patchfiles/0001-nuttx-patch-BMS3.4.patch

configure the BMS
Either use:

tools/configure.sh -e rddrone-bms772:bmsdebug

Or 

tools/configure.sh -e rddrone-bms772

Make the binary with: 

make
