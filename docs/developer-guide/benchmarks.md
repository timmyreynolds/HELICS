# HELICS Benchmarks

The HELICS repository has a few benchmarks that are intendended to test various aspects of the code and record performance over time

## Baseline benchmarks

### ActionMessage
    microbenchmarks to test some operations concerning the serialization of the underlying message structure in HELICS
    
### Conversion
    microbenchmarks to test the serialization and deserialization of common data types in HELICS
    
## Simulation benchmarks

### Echo
    a set of federates representing a hub and spoke model of communication for value based interfaces
    
### Echo Message
    a set of federates representing a hub and spoke model of communication for message based interfaces
    
### Filter
    a variant of the Echo message test that add filters to the messages
    
### ring Benchmark 
    a ring like structure that passes a value token around a bunch of times
    
### timing Benchmark
    similar to echo but doesn't actually send any data just pure test of the timing messages
    
## Message Benchmarks
benchmarks testing various aspects of the messaging structure in HELICS

### MessageLookup
Benchmarks sends messages to random federates, varying the total number of interfaces and federates.

### MessageSend
sending messages between 2 federates varying the message size and count

## Standardized Tests

### PHold
    a standard PHOLD benchmark varying the number of federates.
    
## Multinode Benchmarks

Some of the benchmarks above have multinode variants. These benchmarks will have a standalone binary for the federate used in the benchmark that can be run on each node. Any multinode benchmark run will require some setup to make it launch in your particular environment and knowing the basics for the job scheduler on your cluster will be very helpful.

Any sbatch files for multinode benchmark runs in the repository are setup for running in the pdebug queue on LC's Quartz cluster. They are unlikely to work as is on other clusters, however they should work as a starting point for other clusters using slurm. The minimum changes required are likely to involve setting the queue/partition correctly and ensuring the right bank/account for charging CPU time is used.