Configs to check:
	CONFIG_SYSFS = y
	CONFIG_UNUSED_SYMBOLS = EXPLICITLY NOT SET
	CONFIG_MODULE_SIG = EXPLICITLY NOT SET
	CONFIG_GENERIC_BUG = y
	CONFIG_KALLSYMS = y
	CONFIG_SMP = y
	CONFIG_TRACEPOINTS = y
	HAVE_JUMP_LABEL = defined in include/linux/jump_label.h which IS included
	CONFIG_TRACING = y
	CONFIG_EVENT_TRACING = y
	CONFIG_FTRACE_MCOUNT_RECORD = y
	CONFIG_LIVEPATCH = NOT FOUND
	CONFIG_MODULE_UNLOAD = y
	CONFIG_CONSTRUCTORS = NOT FOUND
	
	
struct module {
	enum module_state state;

	/* Member of list of modules */
	struct list_head list;

	/* Unique handle for this module */
	char name[MODULE_NAME_LEN];

	/* Sysfs stuff. */
	struct module_kobject mkobj;
	struct module_attribute *modinfo_attrs;
	const char *version;
	const char *srcversion;
	struct kobject *holders_dir;

	/* Exported symbols */
	const struct kernel_symbol *syms;
	const unsigned long *crcs;
	unsigned int num_syms;

	/* Kernel parameters. */
#ifdef CONFIG_SYSFS
	struct mutex param_lock;
#endif
	struct kernel_param *kp;
	unsigned int num_kp;

	/* GPL-only exported symbols. */
	unsigned int num_gpl_syms;
	const struct kernel_symbol *gpl_syms;
	const unsigned long *gpl_crcs;

#ifdef CONFIG_UNUSED_SYMBOLS
	/* unused exported symbols. */
	const struct kernel_symbol *unused_syms;
	const unsigned long *unused_crcs;
	unsigned int num_unused_syms;

	/* GPL-only, unused exported symbols. */
	unsigned int num_unused_gpl_syms;
	const struct kernel_symbol *unused_gpl_syms;
	const unsigned long *unused_gpl_crcs;
#endif

#ifdef CONFIG_MODULE_SIG
	/* Signature was verified. */
	bool sig_ok;
#endif

	bool async_probe_requested;

	/* symbols that will be GPL-only in the near future. */
	const struct kernel_symbol *gpl_future_syms;
	const unsigned long *gpl_future_crcs;
	unsigned int num_gpl_future_syms;

	/* Exception table */
	unsigned int num_exentries;
	struct exception_table_entry *extable;

	/* Startup function. */
	int (*init)(void);

	/* Core layout: rbtree is accessed frequently, so keep together. */
	struct module_layout core_layout __module_layout_align;
	struct module_layout init_layout;

	/* Arch-specific module values */
	struct mod_arch_specific arch;

	unsigned int taints;	/* same bits as kernel:tainted */

#ifdef CONFIG_GENERIC_BUG
	/* Support for BUG */
	unsigned num_bugs;
	struct list_head bug_list;
	struct bug_entry *bug_table;
#endif

#ifdef CONFIG_KALLSYMS
	/* Protected by RCU and/or module_mutex: use rcu_dereference() */
	struct mod_kallsyms *kallsyms;
	struct mod_kallsyms core_kallsyms;
	
	/* Section attributes */
	struct module_sect_attrs *sect_attrs;

	/* Notes attributes */
	struct module_notes_attrs *notes_attrs;
#endif

	/* The command line arguments (may be mangled).  People like
	   keeping pointers to this stuff */
	char *args;

#ifdef CONFIG_SMP
	/* Per-cpu data. */
	void __percpu *percpu;
	unsigned int percpu_size;
#endif

#ifdef CONFIG_TRACEPOINTS
	unsigned int num_tracepoints;
	struct tracepoint * const *tracepoints_ptrs;
#endif
#ifdef HAVE_JUMP_LABEL
	struct jump_entry *jump_entries;
	unsigned int num_jump_entries;
#endif
#ifdef CONFIG_TRACING
	unsigned int num_trace_bprintk_fmt;
	const char **trace_bprintk_fmt_start;
#endif
#ifdef CONFIG_EVENT_TRACING
	struct trace_event_call **trace_events;
	unsigned int num_trace_events;
	struct trace_enum_map **trace_enums;
	unsigned int num_trace_enums;
#endif
#ifdef CONFIG_FTRACE_MCOUNT_RECORD
	unsigned int num_ftrace_callsites;
	unsigned long *ftrace_callsites;
#endif

#ifdef CONFIG_LIVEPATCH
	bool klp; /* Is this a livepatch module? */
	bool klp_alive;

	/* Elf information */
	struct klp_modinfo *klp_info;
#endif

#ifdef CONFIG_MODULE_UNLOAD
	/* What modules depend on me? */
	struct list_head source_list;
	/* What modules do I depend on? */
	struct list_head target_list;

	/* Destruction function. */
	void (*exit)(void);

	atomic_t refcnt;
#endif

#ifdef CONFIG_CONSTRUCTORS
	/* Constructor functions. */
	ctor_fn_t *ctors;
	unsigned int num_ctors;
#endif
}