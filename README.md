# typec
C/C++ reflection / visitor generator

Example:

```sh
$ typec -f transform.type > transform.type.h
```

transform.type
```c
extern quat;
extern vec3;

struct Transform
{
	protected:
		vec3 translation;
		quat rotation;
		vec3 scale;
};
```

transform.type.h
```c
...

typedef struct
{
	vec3 translation;
	quat rotation;
	vec3 scale;
} Transform; //struct 0, total index: 0

typedef enum
{
	k_ETypeTransform = 0,
} k_EType;
...

static int vt_fn_visit_type_0_field_0(Transform *inst, TypeFieldVisitor *visitor, void *ctx)
{
	int changed_count = 0;
	if(visitor->visit_vec3)
	{
		return visitor->visit_vec3(ctx, NULL, (vec3*)&inst->translation, 1);
	}
	else if(visitor->visit)
	{
		return visitor->visit(ctx, NULL, (void*)&inst->translation, sizeof(vec3), 1);
	}
	return changed_count;
}
...

...
static int vt_fn_visit_type_0_(void *data, TypeFieldVisitor *visitor, void *ctx, const char *field_name)
{
	Transform *inst = (Transform*)data;
	int changed_count = 0;
	changed_count += vt_fn_visit_type_0_field_0(inst, visitor, ctx);
	changed_count += vt_fn_visit_type_0_field_1(inst, visitor, ctx);
	changed_count += vt_fn_visit_type_0_field_2(inst, visitor, ctx);
	return changed_count;
}
...

```
