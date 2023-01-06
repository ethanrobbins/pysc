import pandas as pd
import numpy as np
import sqlite3
import argparse
import pdb, sys, traceback
import matplotlib.pyplot as plt

def main():
    plt.close('all')
 
    parser = argparse.ArgumentParser(description="Generate perormance reports")
    parser.add_argument('--src', type=str, help="Source sqlite datebase")

    args = parser.parse_args()

    perf_metric = "runtime"
    vars = ["cache__CACHE_ENTRIES", "cache__CACHE_ASSOCIATIVITY", "mem__MEM_NS", "cache_ws"]


    con = sqlite3.connect(args.src)
    df = pd.read_sql_query("SELECT * from perf", con)
    max_runtime = df['runtime'].max()
    max_events = max(df['cache_fill'].max(), df['cache_flush'].max())

    df['cache_size'] = df['cache_ws'] * df['cache__CACHE_ENTRIES']

    var_vals = {}
    for k in vars:
        var_vals[k] = df[k].unique()
        print("  %s  %s"%(k,var_vals[k]))

    print(df.columns)

    # how does memory bandwidth vs cache size look
    _, axes_perf = plt.subplots(nrows=len(var_vals['mem__MEM_NS']),ncols=len(var_vals['cache_ws']))
    _, axes_power = plt.subplots(nrows=len(var_vals['mem__MEM_NS']),ncols=len(var_vals['cache_ws']))
 
    for mem_ns_idx in range(len(var_vals['mem__MEM_NS'])):
        for cache_ws_idx in range(len(var_vals['cache_ws'])):
            subdf = df[(df['mem__MEM_NS']==var_vals['mem__MEM_NS'][mem_ns_idx]) & (df['cache_ws']==var_vals['cache_ws'][cache_ws_idx])]
            sub_perf = subdf.pivot_table(index=["cache_size"], columns=["cache__CACHE_ASSOCIATIVITY"], values=perf_metric)
            sub_power = subdf.pivot_table(index=["cache_size"], columns=["cache__CACHE_ASSOCIATIVITY"], values=["cache_fill","cache_flush"])

            axes_perf[mem_ns_idx,cache_ws_idx].set_ylabel("Runtime")
            axes_perf[mem_ns_idx,cache_ws_idx].set_ylim(0,max_runtime)
            axes_perf[mem_ns_idx,cache_ws_idx].set_xlabel("Cache Size (Bytes)")
            axes_perf[mem_ns_idx,cache_ws_idx].set_title("Cache Line:%d Mem_ns:%d"%(var_vals['cache_ws'][cache_ws_idx],var_vals['mem__MEM_NS'][mem_ns_idx]))
            sub_perf.plot.bar(ax=axes_perf[mem_ns_idx,cache_ws_idx]).grid(axis='y')

            axes_power[mem_ns_idx,cache_ws_idx].set_ylabel("Events")
            axes_power[mem_ns_idx,cache_ws_idx].set_ylim(0,max_events)
            axes_power[mem_ns_idx,cache_ws_idx].set_xlabel("Cache Size (Bytes)")
            axes_power[mem_ns_idx,cache_ws_idx].set_title("Cache Line:%d Mem_ns:%d"%(var_vals['cache_ws'][cache_ws_idx],var_vals['mem__MEM_NS'][mem_ns_idx]))
            sub_power.plot.bar(ax=axes_power[mem_ns_idx,cache_ws_idx]).grid(axis='y')

 
    plt.show()


    con.close()


if __name__ == "__main__":
    try:
        main()
    except Exception:
        exc_type, exc_value, exc_traceback = sys.exc_info()
        print("*** print_tb:")
        traceback.print_tb(exc_traceback)
        traceback.print_exc()
        pdb.post_mortem(exc_traceback)
