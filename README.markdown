Low-Level Data Structure
========================
llds provides a low-level data structure to user space. llds attempts to maximize memory efficiency via bypassing the virtual memory layer and optimized data structure alignment.

The llds general working thesis is: for large memory applications, the virtual memory layer can hurt application performance because of increased memory latency when dealing with large data structures.

Applicable use cases: applications on systems that utilize large in-memory data structures (>16GB). llds is not for suitable for applications which do not have a a large singular data structure, 16GB or smaller, there will be no significant gains in such use cases. 

Installing/Configuring
======================

<pre>
cmake .
make
</pre>

How it Works
============
llds is a Linux kernel module (2.6, 3.0) that leverages the ubiquitous red-black tree data structure within the kernel. Red-black tree data structures are highly optimized in the kernel and are used in managing processes to epoll file descriptors to file systems.

Memory management in llds is optimized for traversal latency, not space efficiency, though space savings are probable due to better alignment in most cases. llds data structures should not consume any more memory than equivalent user space data structures.

Traversal latency is optimized by exploiting underlying physical RAM mechanics, avoiding CPU cache pollution, and streamlining CPU data prefetching. The basic working theorem behind decreased latency is: continuous memory in physical space is more efficient than continuous memory in virtual space (which is not guaranteed to be continous in physical space). Fragmented memory access is less efficient when interacting with DRAM controllers. The efficiency also further suffers on NUMA systems as the number of processors/memory banks increases.

libforrest
==========
Developers can interact directly with the llds chardev using ioctl(2), however, it is highly recommended that the libforrest API is used to avoid incompatibilities should the ioctl interface change in the future. 

libforrest provides the basic key-value store operations: get, set, and delete. In addition, it provides a 64-bit MurmurHash (rev. A) for llds key hashing.

Examples are provided in the libforrest/examples directory.

Benchmarks
==========
Benchmarks are inherently fluid. All samples and timings are available at http://github.com/johnj/llds-benchmarks, additionally there is a `run_tests.sh` script provided which utilizes oprofile. Along with the `run_tests.sh` script, there is a user-space implementation of red-black trees and an equivalent llds implementation. The goal of benchmarking is about opining to the results of a particular environment but all the tools and scripts are available to let users test their own mileage.

Benchmark environment: Dell PowerEdge R610, 4x Intel Xeon L5640 (Westmere) w/HT (24 cores), 192GB DDR3 DRAM, Ubuntu 10.04.3 LTS. The keys are 64-bit integers and the values are incremented strings (ie, "0", "1", "2"..."N"). There were no major page faults.

For conciseness, only tests with 2/16/24 threads and 500K/1.5M/2M keys are listed. dmidecode, samples, and full benchmarks are available at http://github.com/johnj/llds-benchmarks

Wall Timings (in seconds)
-------------------------
<table>
<tr><td>Threads</td><td># of Items<td>userspace</td><td>llds</td><td>llds improvement</td></tr>
<tr><td>2</td><td>500000000></td><td>3564</td><td>1761</td><td>2.02x</td></tr>
<tr><td>16</td><td>1500000000></td><td>9291</td><td>4112</td><td>2.26x</td></tr>
<tr><td>24</td><td>2000000000></td><td>12645</td><td>5670</td><td>2.23x</td></tr>
</table>

Unhalted CPU cycles (10000 cycles @ 133mHz)
-----------------------------------------
<table>
<tr><td>Threads</td><td># of Items</td><td>userspace</td><td>llds</td></tr>
<tr><td>2</td><td>500000000</td><td>87418776</td><td>377458531</td></tr>
<tr><td>16</td><td>1500000000</td><td>279203932</td><td>5107099682</td></tr>
<tr><td>24</td><td>2000000000</td><td>968091233</td><td>5529234102</td></tr>
</table>

L1 cache hits (200000 per sample)
----------------------------------
<table>
<tr><td>Threads</td><td># of Items</td><td>userspace</td><td>llds</td><td>llds improvement</td></tr>
<tr><td>2</td><td>500000000</td><td>3077671</td><td>5502292</td><td>1.78x</td></tr>
<tr><td>16</td><td>1500000000</td><td>15120921</td><td>27231553</td><td>1.80x</td></tr>
<tr><td>24</td><td>2000000000</td><td>23746988</td><td>39196177</td><td>1.65x</td></tr>
</table>

L2 cache hits (200000 per sample)
----------------------------------
<table>
<tr><td>Threads</td><td># of Items</td><td>userspace</td><td>llds</td><td>llds improvement</td></tr>
<tr><td>2</td><td>500000000</td><td>21866</td><td>60214</td><td>2.75x</td></tr>
<tr><td>16</td><td>1500000000</td><td>82101</td><td>511285</td><td>6.23x</td></tr>
<tr><td>24</td><td>2000000000</td><td>127072</td><td>800846</td><td>6.30x</td></tr>
</table>

L3/Last-Level cache hits (200000 per sample)
---------------------------------------------
<table>
<tr><td>Threads</td><td># of Items</td><td>userspace</td><td>llds</td><td>llds improvement</td></tr>
<tr><td>2</td><td>500000000</td><td>26069</td><td>32259</td><td>1.24x</td></tr>
<tr><td>16</td><td>1500000000</td><td>148827</td><td>254562</td><td>1.71x</td></tr>
<tr><td>24</td><td>2000000000</td><td>270191</td><td>341649</td><td>1.26x</td></tr>
</table>

L1 Data Prefetch misses (200000 per *hardware* sample)
---------------------------------------------
<table>
<tr><td>Threads</td><td># of Items</td><td>userspace</td><td>llds</td><td>llds improvement</td></tr>
<tr><td>2</td><td>500000000</td><td>52396</td><td>21113</td><td>2.48x</td></tr>
<tr><td>16</td><td>1500000000</td><td>350753</td><td>120891</td><td>2.90x</td></tr>
<tr><td>24</td><td>2000000000</td><td>544791</td><td>210268</td><td>2.59x</td></tr>
</table>


Status
======
llds is experimental. Though it's been tested in various environments (including integration into a search engine) it is not known to be in use on any production system, yet. With additional resources (eyes) looking at llds, the hope is that llds will be stable by Q4 '12 (ala Wall, Perl6, and Christmas).

Known Limitations/Issues
========================
libforrest has a limit on the value which comes back from kernel space, the default is 4096 bytes, it can be adjusted through the FORREST_MAX_VAL_LEN directive at compile time.

Future Work
===========
- Support for additional data structures (hashes are questionable)
- Add atomic operations (increment, decrement, CAS, etc.) in libforrest and llds
- Research paper about the virtual memory overhead & implementation in the kernel with mitigation techniques