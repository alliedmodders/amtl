# vim: set ts=2 sw=2 tw=99 noet ft=python: 
# 
# Copyright (C) 2004-2012 David Anderson
# 
# This file is part of SourcePawn.
# 
# SourcePawn is free software: you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option)
# any later version.
# 
# SourcePawn is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License along with
# SourcePawn. If not, see http://www.gnu.org/licenses/.
#
import sys
import ambuild.runner as runner

run = runner.Runner()
run.options.add_option('--enable-debug', action='store_const', const='1', dest='debug',
                       help='Enable debugging symbols')
run.options.add_option('--enable-optimize', action='store_const', const='1', dest='opt',
                       help='Enable optimization')
run.options.add_option('--arch', dest='arch', help='Architecture (x86, x64)')
run.Configure(sys.path[0])
