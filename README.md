# Order-Book

A high speed NASDAQ ITCH trade data parser and Limit Order Book.

This project will serve as a playground for exploring C++ performance and efficiency ideas.

# Dependencies

- [Boost.Pool](https://www.boost.org/doc/libs/1_75_0/libs/pool/doc/html/index.html)

A memory pool library for C++.

`apt-get install libboost-all-dev`

- [sparsehash](https://github.com/sparsehash/sparsehash)

`std::unordered_map` alternative.

- [glog](https://github.com/google/glog)

Logging library with debug-only logs/asserts (though I have my doubts).

`apt-get install libgoogle-glog-dev`

# Build and Usage

- Build with `make rel`
- Run with `./order-book <NASDAQ_ITCH_50_file>`

~~For benchmarking, comment out all `DLOG`s in `src/order_book.cpp` as `DLOG_ASSERT` seems to be enforced even with `#define NDEBUG`.~~ For benchmarking, the `DLOG_ASSERT` issue was [recently resolved](https://github.com/google/glog/pull/1035), just make sure you have the latest `glog` build. Otherwise, comment out all `DLOG*`s for benchmarking (`:%s/DLOG_ASSERT/\/\/DLOG_ASSERT/` in `vim`).

# Design

The Order Book follows a price/time priority (i.e. best price first, then earliest order).

- Order books are stored in a hash map keyed by symbol
- Levels (limit prices) are stored in a hash map keyed by limit price
- Orders are stored in a hash map keyed by reference number

In order to maintain the price/time priority:
- Levels are also stored in a sorted tree (so adding orders with new limits is O(logm) where m = # of limits). The tree provides access to Level 1 data in O(1).
- Orders of the same limit price form a doubly linked list. This allows for time ordering where new orders are simply appended in O(1). The orders being doubly linked allows for O(1) deletion since the Order can just be referenced from the hash map.

# Performance Considerations

### ITCH Reader

The ITCH 5.0 data files are +10GB binary files.

- to avoid excessive I/O, the files are read in large chunks at a time
- Memory is only allocated once and reused when storing these chunks

The file reader itself processes +100 million messages per second and has very little overhead.

```
aanrv@debianssd:~/Documents/Order-Book$ ./order-book spec/12302019.NASDAQ_ITCH50  
Processing spec/12302019.NASDAQ_ITCH50
processed 268744780 messages (8251407909 bytes) in 2052 milliseconds
```

  ![image](https://github.com/aanrv/Order-Book/assets/14251976/67a16730-9049-4566-b1c4-d3a73e2e5658)

### Order Book

The Order Book takes up the majority of the application's runtime.

- The bulk of the operations are Adds and Deletes with Replaces at a distant 3rd, so this is where the optimizations were focused

```
aanrv@debianssd:~/Documents/Order-Book$ ./order-book spec/12302019.NASDAQ_ITCH50  
Processing spec/12302019.NASDAQ_ITCH50
Add: 118631456
Exc: 5822741
Cnl: 2787676
Del: 114360997
Rpl: 21639067
```

- To avoid the overhead costs of allocating/deallocating orders hundreds of millions of times, Boost's memory pool ([Boost.Pool](https://www.boost.org/doc/libs/1_75_0/libs/pool/doc/html/boost_pool/pool/interfaces.html)) was used. Seems to have removed memory management as a major bottleneck.

![image](https://github.com/aanrv/Order-Book/assets/14251976/fdcb4bf4-ab87-426f-8b97-75da155ad8c6)

- Levels require a sorted tree so that L1 info is available in O(1), so adding an order with a new limit is O(logm). But # of limits < # of orders by far (average of ~5 orders per limit), and this doesn't seem to be a bottleneck based on profiling.

```
aanrv@debianssd:~/Documents/Order-Book$ ./order-book spec/12302019.NASDAQ_ITCH50  
Processing spec/12302019.NASDAQ_ITCH50
# Levels: 30113162
# Orders: 140270523
```

- The bulk of the time was spent on hash map insertion/deletion operations. Replacing `std::unordered_map` with `google::dense_hash_map` yielded a >30% decrease in runtime. Tested on an i7-9750H 2.6 GHz laptop in VMWare:

```
aanrv@debianssd:~/Documents/Order-Book$ ./order-book spec/12302019.NASDAQ_ITCH50  
Using: std::unordered_map
Processing spec/12302019.NASDAQ_ITCH50
processed 268744780 messages (8251407909 bytes) in 216493 milliseconds
```

```
aanrv@debianssd:~/Documents/Order-Book$ ./order-book spec/12302019.NASDAQ_ITCH50
Using: google::dense_hash_map
Processing spec/12302019.NASDAQ_ITCH50
processed 268744780 messages (8251407909 bytes) in 148342 milliseconds
```

# Further Improvements

- ITCH file is read in chunks and is fairly fast, but look into [memory mapped files](https://stackoverflow.com/a/17925143/2014342) as well
- `object_pool` was used as the memory pool. Look into other memory pools which may be for suitable for this use case (e.g. where I can allocate memory in advance)
- Perhaps there is a faster `std::map` alternative
- Profiling still shows hash map insertions/deletions to be the largest bottleneck:

![image](https://github.com/aanrv/Order-Book/assets/14251976/1a7415f2-5ae7-41d0-bd1a-3836cdd37dd7)

Even though switching to `google::dense_hash_map` significantly reduced runtime, look into further alternatives. Perhaps the most significant improvement would come from finding a way to use a contiguous container rather than a map/tree. This idea is discussed in detail [here](https://quant.stackexchange.com/questions/3783/what-is-an-efficient-data-structure-to-model-order-book/32482#32482). 

# Resources

- [CppCon 2017: Carl Cook “When a Microsecond Is an Eternity: High Performance Trading Systems in C++”](https://www.youtube.com/watch?v=NH1Tta7purM)
- [CppCon 2016: Timur Doumler “Want fast C++? Know your hardware!"](https://www.youtube.com/watch?v=BP6NxVxDQIs)
- https://quant.stackexchange.com/questions/3783/what-is-an-efficient-data-structure-to-model-order-book/32482#32482
- https://www.boost.org/doc/libs/1_75_0/libs/pool/doc/html/boost_pool/pool/pooling.html
- [Sample NASDAQ ITCH data files](https://emi.nasdaq.com/ITCH/Nasdaq%20ITCH/)
- [ITCH 5.0 Spec](https://www.nasdaqtrader.com/content/technicalsupport/specifications/dataproducts/NQTVITCHSpecification.pdf)
- [BinaryFILE 1.0 Spec](https://www.nasdaqtrader.com/content/technicalSupport/specifications/dataproducts/binaryfile.pdf)
