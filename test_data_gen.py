import random
import struct
import sys
import getopt

def help_msg():
    print("Usage: {0} [options]".format(sys.argv[0]))
    print("")
    print(" -N num    Number of test tuples to generate, default: 100k")
    print(" -b num    Number of bytes in a machine word for the SUT")
    print("           Use \"4\" for 32 bit systems and \"8\" for 64 bit")
    print("           systems. Default: 8")
    print(" -n num    Number of machine words to use for generated numbers")
    print("           Default: 32 (i.e. 2048 bit on 64-bit machine)")
    print(" -2 num    Number of machine words to use for mod operation output")
    print("           Default: 32")
    print(" -l file   Name of output log file, \"log.csv\" by default")
    print(" -o file   Name of output file with generated data.")
    print("           \"data.bin\" by default")
    print(" --add     Generate data for addition tests (default)")
    print(" --sub     Generate data for subtraction tests")
    print(" --mul     Generate data for multiplication tests")
    print(" --mod     Generate data for modulo tests")
    print(" -h        This message")

limb_size = 8
nlimb = 32  # operate on 2048 bit numbers
mod_limb = 32

N = 100000
log_out = "log.csv"
data_out = "data.bin"
oper = "add"

log_file = open(log_out, "w")
data_file = open(data_out, "wb")

argv = sys.argv[1:]

opts, args = getopt.getopt(argv, "N:b:n:2:l:o:h", ["add", "sub", "mul", "mod"])
for opt, arg in opts:
    if opt == "-h":
        help_msg()
        sys.exit(0)
    elif opt == "-N":
        N = int(arg)
    elif opt == "-b":
        limb_size = int(arg)
    elif opt == "-n":
        nlimb = int(arg)
    elif opt == "-2":
        modlimb = int(arg)
    elif opt == "-l":
        log_out = arg
    elif opt == "-o":
        data_out = arg
    elif opt == "--add":
        oper = "add"
    elif opt == "--sub":
        oper = "sub"
    elif opt == "--mul":
        oper = "mul"
    elif opt == "--mod":
        oper = "mod"
    else:
        print("Unrecognised option: {0}".format(opt))
        help_msg()
        sys.exit(1)

if args:
    print("Trailing arguments: {0}".format(args))
    help_msg()
    sys.exit(1)

# make a modulus the count of numbers that can be represented in this many bits
# for the input
mod = 2**(8*limb_size*nlimb)
# for the mod op output
mod_mod = 2**(8*limb_size*mod_limb)

probes = [
        ("null-null", (0, 0)),
        ("null-byte", (0, 0xff)),
        ("byte-null", (0xff, 0)),
        ("byte-byte", (0xff, 0xff)),
        ("qword-one", (2**(8*limb_size)-1, 1)),
        ("qword-qword", (2**(8*limb_size)-1, 2**(8*limb_size)-1)),
        ("mod-1-null", (mod-1, 0)),
        ("null-mod-1", (0, mod-1)),
        ("mod-1-one", (mod-1, 1)),
        # propagagation over all possible limbs
        ("mod-1-mod-1", (mod-1, mod-1)),
        # some carry propagation in low limbs:
        ]
if nlimb >= 4:
    probes += [
        ("qqword-qqword", (2**(8*limb_size*4)-1, 2**(8*limb_size*4)-1)),
        ]
if nlimb >= 3:
    probes += [
        # carry propagation in mid limbs:
        ("mid-mid", ((2**(8*limb_size*(nlimb//3))-1)<<(8*limb_size*(nlimb*2//3)),
                     (2**(8*limb_size*(nlimb//3))-1)<<(8*limb_size*(nlimb*2//3)))),
        # carry propagation in high limbs:
        ("high-high", (2**(8*limb_size*nlimb)-1-(2**(8*limb_size*(nlimb*2//3))-1),
                       2**(8*limb_size*nlimb)-1-(2**(8*limb_size*(nlimb*2//3))-1))),
        ]

mod_probes = [
        ("null-mod-1", (0, mod_mod-1)),
        ("one-mod-1", (1, mod_mod-1)),
        ("qword-mod-1", (2**(8*limb_size)-1, mod_mod-1)),
        ("mod-2-mod-1", (mod_mod-2, mod_mod-1)),
        ("max-mod-1", (mod-1, mod_mod-1)),
        ("mod-1-sq-mod-1", (((mod_mod-1)**2)%(mod-1), mod_mod-1)),
        ("null-mod>>limb", (0, mod_mod >> (limb_size * 8))),
        ("null-mod-1>>limb", (0, (mod_mod - 1) >> (limb_size * 8))),
        ("null-mod>>2*limb", (0, mod_mod >> (limb_size * 8 * 2))),
        ("null-mod-1>>2*limb", (0, (mod_mod - 1) >> (limb_size * 8 * 2))),
        ("max-mod>>limb", (mod-1, mod_mod >> (limb_size * 8))),
        ("max-mod-1>>limb", (mod-1, (mod_mod - 1) >> (limb_size * 8))),
        ("max-mod>>2*limb", (mod-1, mod_mod >> (limb_size * 8 * 2))),
        ("max-mod-1>>2*limb", (mod-1, (mod_mod - 1) >> (limb_size * 8 * 2))),
        ]

if oper == "mod":
    probes = mod_probes

log_file.write(",".join(name for name, _ in probes))
log_file.write("\n")

rand = random.SystemRandom()

o = [None] * len(probes)
for _ in range(N):
    order = random.sample(range(len(probes)), len(probes))
    for i, pos in enumerate(order):
        val_a, val_b = probes[pos][1]
        # write inputs to the file
        assert val_a < mod
        data_file.write(val_a.to_bytes(length=limb_size*nlimb, byteorder="big"))
        if oper == "mod":
            assert val_b < mod_mod
            data_file.write(val_b.to_bytes(length=limb_size*mod_limb, byteorder="big"))
        else:
            assert val_b < mod
            data_file.write(val_b.to_bytes(length=limb_size*nlimb, byteorder="big"))
        # write expected output to the file
        if oper == "add":
            data_file.write(((val_a + val_b) & (mod-1)).to_bytes(length=limb_size*nlimb, byteorder="big"))
        elif oper == "mul":
            data_file.write((val_a * val_b).to_bytes(length=limb_size*nlimb*2, byteorder="big"))
        elif oper == "mod":
            data_file.write((val_a % val_b).to_bytes(length=limb_size*mod_limb, byteorder="big"))
        else:
            assert oper == "sub"
            data_file.write(((val_a - val_b) & (mod-1)).to_bytes(length=limb_size*nlimb, byteorder="big"))
        o[pos] = i

    # in log we need to write the position of probe in tuple, not
    # order in which they were executed
    log_file.write(",".join(str(i) for i in o))
    log_file.write("\n")

log_file.close()
data_file.close()
