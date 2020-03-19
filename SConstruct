# \brief	SConstruct file for Open Micron
# \author	Circuit Chaos
# \date		2020-03-12

import subprocess

def getGitHash():
    return subprocess.Popen('git rev-parse --short HEAD', stdout=subprocess.PIPE, shell=True).stdout.read().strip()

env = Environment()
env['CCFLAGS']	= '-Wall -Wextra -std=c++11 -O2 -g -DGIT_HASH=' + getGitHash()
env['CPPPATH']	= 'src'
env['LIBS'] = 'csv'

env.VariantDir('build', 'src', duplicate = 0)
env.AlwaysBuild('build/version.o')
omi = env.Program('build/omi', Glob('build/*.cpp'))

env.Install('/usr/local/bin', omi)

# env.Install('/usr/local/man/man1', 'doc/omi.1')
# env.Alias('install', ['/usr/local/bin', '/usr/local/man/man1'])
env.Alias('install', '/usr/local/bin')
