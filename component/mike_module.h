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

	struct mutex param_lock;

	struct kernel_param *kp;
	unsigned int num_kp;

	/* GPL-only exported symbols. */
	unsigned int num_gpl_syms;
	const struct kernel_symbol *gpl_syms;
	const unsigned long *gpl_crcs;


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


	/* Support for BUG */
	unsigned num_bugs;
	struct list_head bug_list;
	struct bug_entry *bug_table;

	/* Protected by RCU and/or module_mutex: use rcu_dereference() */
	struct mod_kallsyms *kallsyms;
	struct mod_kallsyms core_kallsyms;
	
	/* Section attributes */
	struct module_sect_attrs *sect_attrs;

	/* Notes attributes */
	struct module_notes_attrs *notes_attrs;


	/* The command line arguments (may be mangled).  People like
	   keeping pointers to this stuff */
	char *args;

	/* Per-cpu data. */
	void __percpu *percpu;
	unsigned int percpu_size;

	unsigned int num_tracepoints;
	struct tracepoint * const *tracepoints_ptrs;

	struct jump_entry *jump_entries;
	unsigned int num_jump_entries;

	unsigned int num_trace_bprintk_fmt;
	const char **trace_bprintk_fmt_start;

	struct trace_event_call **trace_events;
	unsigned int num_trace_events;
	struct trace_enum_map **trace_enums;
	unsigned int num_trace_enums;

	unsigned int num_ftrace_callsites;
	unsigned long *ftrace_callsites;

	/* What modules depend on me? */
	struct list_head source_list;
	/* What modules do I depend on? */
	struct list_head target_list;

	/* Destruction function. */
	void (*exit)(void);

	atomic_t refcnt;

}