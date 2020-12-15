#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <GLUT/glut.h>

#define WINDOW_SIZE_W 1024
#define WINDOW_SIZE_H 768
#define INPUT_BUF_SIZE 1024

/**
 * 关系矩阵数据结构
 */
struct relation_matrix_ {
    size_t n; // 元素个数
    _Bool* p; // nxn的关系矩阵起始位置指针
};
typedef struct relation_matrix_ relation_matrix_t;
#define GET_EL(m, r, c) m.p[(r) * m.n + (c)]

/**
 * 检查是否为偏序关系
 * @param m 关系矩阵
 *
 * @return NO_VIOLATION（如果为偏序关系），NO_REFLEXIVE（不具备自反性），NO_ANTI_SYMMETRIC（不具备反对称性），NO_TRANSITIVE（不具备传递性）
 */
void check_porder(relation_matrix_t m);

/**
 * 获取邻接表存储的覆盖边集合，直接将关系矩阵中的非覆盖关系删去，得到的关系矩阵作为覆盖边构成图的邻接矩阵
 * @param m 关系矩阵
 */
void get_cover_edges(relation_matrix_t m);

/**
 * 存储每个元素对应层数的数组
 */
typedef size_t* el_level;

/**
 * 通过拓扑排序计算每个元素所在的层数
 * @param m 关系矩阵（注意：此时仅包含覆盖边）
 * @param level 将层数存储在这一数组中
 */
void calc_level(relation_matrix_t m, el_level level);

size_t max_level = 0;
relation_matrix_t glb_mat;
el_level glb_level;
size_t glb_n;

struct vertex_pos {
    float x, y;
};
typedef struct vertex_pos vertex_pos_t;

#define ISTRING_HASH_MOD 1023

struct istring_ {
    const char* str;
    size_t len;
    size_t id;
};
typedef struct istring_ istring_t;
size_t istring_hash(const char* str, size_t len);
istring_t new_istring(const char* str, size_t len, size_t id);
istring_t lookup_istring(const char* str, size_t len);

struct istring_bucket_ {
    istring_t is;
    struct istring_bucket_* next;
};

struct istring_bucket_* istring_htable[ISTRING_HASH_MOD] = {NULL};

char ibuf[INPUT_BUF_SIZE];
istring_t* vertex2str; // 将顶点转换到字符串

/**
 * 输入数据函数
 */
void input_data() {
    size_t n, m;
    printf("输入元素个数和包含在二元关系内的有序对数，空格隔开：");
    scanf("%ld %ld", &n, &m);
    vertex2str = (istring_t*) malloc(sizeof(istring_t) * n);
    relation_matrix_t mat = {
            .n = n,
            .p = (_Bool*) calloc(n * n, sizeof(_Bool))
    };
    printf("输入集合的每个元素：\n");
    for (size_t i = 0; i < n; i++) {
        scanf("%s", ibuf);
        if (lookup_istring(ibuf, strlen(ibuf)).str) {
            printf("该元素已经存在！\n");
            exit(1);
        }
        vertex2str[i] = new_istring(ibuf, strlen(ibuf), i);
    }
    printf("输入二元关系中所有有序对：\n");
    for (size_t i = 0; i < m; i++) {
        scanf("%s", ibuf);
        istring_t v1 = lookup_istring(ibuf, strlen(ibuf));
        scanf("%s", ibuf);
        istring_t v2 = lookup_istring(ibuf, strlen(ibuf));
        if (!v1.str) {
            printf("元素%s不存在\n", v1.str);
            exit(1);
        }
        if (!v2.str) {
            printf("元素%s不存在\n", v2.str);
            exit(1);
        }
        GET_EL(mat, v1.id, v2.id) = true;
    }

    printf("正在检查该关系是否符合偏序关系...\n");
    check_porder(mat);
    printf("检查成功，符合偏序关系！\n");

    printf("正在获取覆盖边...\n");
    get_cover_edges(mat);
    for (size_t i = 0; i < n; i++) {
        for (size_t j = 0; j < n; j++) {
            if (!GET_EL(mat, i, j)) {
                continue;
            }
            printf("%ld %ld\n", i, j);
        }
    }

    printf("正在通过拓扑排序计算每个元素所在的层数...\n");
    el_level level = (el_level) malloc(sizeof(size_t) * n);
    calc_level(mat, level);
    for (size_t i = 0; i < n; i++) {
        printf("元素%ld的层数为%ld\n", i, level[i]);
        if (level[i] > max_level) {
            max_level = level[i];
        }
    }

    glb_mat = mat;
    glb_level = level;
    glb_n = n;
}

/**
 * 可视化显示函数
 */
void render() {
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    // 先计算每个点的坐标
    vertex_pos_t* pos = (vertex_pos_t*) malloc(sizeof(vertex_pos_t) * glb_n);
    float h = 2.0f / (1.0f + max_level); // 每一层点所占的高度
    for (size_t level = 0; level <= max_level; level++) {
        // 统计层次为level的点数
        size_t cnt = 0;
        for (size_t i = 0; i < glb_n; i++) {
            if (glb_level[i] == level) {
                cnt++;
            }
        }

        // 计算该层每一个点所占的宽度
        float w = 2.0f / cnt;

        // 绘制层次为level的所有点
        size_t off = 0;
        for (size_t i = 0; i < glb_n; i++) {
            if (glb_level[i] != level) {
                continue;
            }

            // 距离屏幕左侧的距离为(0.5 + off) * w，映射到[-1,1]区间，得到横坐标
            pos[i].x = -1.0f + (0.5f + off) * w;
            // 距离屏幕底部的距离为(0.5 + level) * h，映射到[-1,1]区间，得到纵坐标
            pos[i].y = -1.0f + (0.5f + level) * h;

            off++;
        }
    }

    // 然后连线
    glColor3f(1.0f, 1.0f, 1.0f);
    glLineWidth(20);
    for (size_t i = 0; i < glb_n; i++) {
        for (size_t j = 0; j < glb_n; j++) {
            if (!GET_EL(glb_mat, i, j)) {
                continue;
            }

            glBegin(GL_LINES);
            glVertex2f(pos[i].x, pos[i].y);
            glVertex2f(pos[j].x, pos[j].y);
            glEnd();
        }
    }

    // 然后画每一个点
    glPointSize(20);
    for (size_t i = 0; i < glb_n; i++) {
        glColor3f(1.0f, 1.0f, 1.0f);
        glBegin(GL_POINTS);
        glVertex2f(pos[i].x, pos[i].y);
        glEnd();

        // 显示名称
        glColor3f(1.0f, 0.0f, 0.0f);
        glRasterPos2f(pos[i].x + 0.02, pos[i].y);
        // 输出文字
        for (size_t j = 0; j < vertex2str[i].len; j++) {
            glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, vertex2str[i].str[j]);
        }
    }

    free(pos);

    glFlush();
}

int main(int argc, char** argv) {
    input_data();

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(WINDOW_SIZE_W, WINDOW_SIZE_H);
    glutCreateWindow("Hasse graph visualization");
    glutDisplayFunc(render);
    glutMainLoop();
    return 0;
}

void check_porder(relation_matrix_t m) {
    // 检查自反性
    for (size_t i = 0; i < m.n; i++) {
        if (!GET_EL(m, i, i)) {
            printf("错误：不满足自反性。元素：%ld", i);
            exit(1);
        }
    }

    // 检查反对称性
    for (size_t i = 0; i < m.n; i++) {
        for (size_t j = i + 1; j < m.n; j++) {
            if  (GET_EL(m, i, j) && GET_EL(m, j, i)) {
                printf("错误：不满足反对称性。元素：%ld %ld", i, j);
                exit(1);
            }
        }
    }

    // 检查传递性
    for (size_t i = 0; i < m.n; i++) {
        for (size_t j = 0; j < m.n; j++) {
            for (size_t k = 0; k < m.n; k++) {
                if (GET_EL(m, i, k) && GET_EL(m, k, j) && !GET_EL(m, i, j)) {
                    printf("错误：不满足传递性。元素：%ld %ld %ld", i, k, j);
                    exit(1);
                }
            }
        }
    }
}

void get_cover_edges(relation_matrix_t m) {
    // 备份一个原来的关系矩阵
    relation_matrix_t origin = {
            .n = m.n,
            .p = (_Bool*) malloc(m.n * m.n * sizeof(_Bool))
    };
    memcpy(origin.p, m.p, m.n * m.n);

    for (size_t i = 0; i < m.n; i++) {
        for (size_t j = 0; j < m.n; j++) {
            for (size_t k = 0; k < m.n; k++) {
                if (i == j || (k != i && k != j && GET_EL(origin, i, k) && GET_EL(origin, k, j))) {
                    GET_EL(m, i, j) = false;
                    break;
                }
            }
        }
    }

    free(origin.p);
}

void calc_level(relation_matrix_t m, el_level level) {
    // 创建入度数组
    size_t* in_deg = (size_t*) calloc(m.n, sizeof(size_t));

    // 将每个顶点的层次设置为0
    for (size_t i = 0; i < m.n; i++) {
        level[i] = 0;
    }

    // 统计每个顶点的入度
    for (size_t i = 0; i < m.n; i++) {
        for (size_t j = 0; j < m.n; j++) {
            if (GET_EL(m, i, j)) {
                in_deg[j]++;
            }
        }
    }

    // 创建拓扑排序队列。容易证明每个点至多入队一次，所以队列长度开到n足矣
    size_t* top_q = (size_t*) malloc(m.n * sizeof(size_t));
    size_t front = 0, rear = 0;

    // 所有0入度顶点入队
    for (size_t i = 0; i < m.n; i++) {
        if (in_deg[i] == 0) {
            top_q[rear++] = i;
        }
    }

    // 相当于拓扑序上DP
    // f[i]=max{f[j]+1}，其中j为i的前驱结点
    while (front < rear) {
        size_t cur = top_q[front++];
        size_t cur_level = level[cur];
        for (size_t v = 0; v < m.n; v++) {
            if (!GET_EL(m, cur, v)) {
                continue;
            }
            in_deg[v]--;
            if (in_deg[v] == 0) {
                top_q[rear++] = v;
            }
            if (cur_level + 1 > level[v]) {
                level[v] = cur_level + 1;
            }
        }
    }

    free(top_q);

    free(in_deg);
}

istring_t new_istring(const char* str, size_t len, size_t id) {
    istring_t res = {
            .len = len,
            .id = id
    };
    char* p = malloc(len + 1);
    memcpy((void*) p, str, len);
    p[len] = '\0';
    res.str = p;

    size_t h = istring_hash(str, len);
    struct istring_bucket_* term = istring_htable[h];
    istring_htable[h] = malloc(sizeof(struct istring_bucket_));
    istring_htable[h]->next = term;

    return istring_htable[h]->is = res;
}

istring_t lookup_istring(const char* str, size_t len) {
    size_t h = istring_hash(str, len);
    if (istring_htable[h]) {
        struct istring_bucket_* p = istring_htable[h];
        while (p != NULL) {
            if (p->is.len == len && !memcmp(p->is.str, str, len)) {
                return p->is;
            }
            p = p->next;
        }
    }

    istring_t not_found = {
            .str = NULL,
            .len = 0
    };
    return not_found;
}

size_t istring_hash(const char* str, size_t len) {
    size_t h = 0;
    for (size_t p = 0; p < len; p++) {
        h = h * 65599 + str[p];
        h %= ISTRING_HASH_MOD;
    }
    return h;
}
