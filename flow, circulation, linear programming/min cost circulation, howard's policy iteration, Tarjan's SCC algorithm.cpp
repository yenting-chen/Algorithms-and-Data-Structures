/*

min cost circulation, Howard's policy iteration, Tarjan's SCC algorithm

2014 Benelux Algorithm Programming Contest (BAPC 14)
problem A - Avoiding the Apocalypse
https://codeforces.com/gym/101512

*/

#include<cstdio>
#include<algorithm>
#include<vector>
#include<queue>
#include<stack>
using namespace std;

const int INF = (1 << 30) - 1;
const double EPS = 1e-8;

struct edge
{
	int u, v, w, c, x, r;

	edge(int _u = 0, int _v = 0, int _w = 0, int _c = 0, int _x = 0, int _r = 0)
	{
		u = _u, v = _v, w = _w, c = _c, x = _x, r = _r;
	}
};

struct SCC
{
	vector<vector<int>> edg;
	vector<int> dfn, low, com;
	stack<int> S;
	int d = 0, cnt = 0;

	SCC(int n, const vector<edge>& e) : edg(n + 1), dfn(n + 1), low(n + 1), com(n + 1)
	{
		for (const edge& E : e)
			edg[E.u].push_back(E.v);

		for (int i = 1; i <= n; i++)
			if (!dfn[i])
				dfs(i);
	}

	void dfs(int u)
	{
		dfn[u] = low[u] = ++d;
		S.push(u);

		for (int v : edg[u])
		{
			if (!dfn[v])
				dfs(v);

			if (!com[v])
				low[u] = min(low[u], low[v]);
		}

		if (dfn[u] == low[u])
		{
			cnt++;
			int v;
			while (v = S.top(), S.pop(), com[v] = cnt, v != u)
				;
		}
	}
};

struct howard
{
	int n;
	vector<edge> e;
	vector<double> d;
	vector<int> c, p, x;

	howard(int _n, const vector<edge> &_e) : n(_n), e(_e), d(n + 1, INF), c(n + 1), p(n + 1), x(n + 1)
	{
	}

	pair<double, vector<int>> operator()()
	{
		for (const edge& E : e)
		{
			if (d[E.u] > E.c)
			{
				d[E.u] = c[E.u] = E.c;
				p[E.u] = E.v;
				x[E.u] = E.x;
			}
		}

		for (; ; )
		{
			double ld;
			int s;
			get_cycle(ld, s);
			reverse_dfs(ld, s);

			if (!improve(ld))
			{
				get_cycle(ld, s);

				vector<int> C;
				C.push_back(x[s]);
				for (int u = p[s]; u != s; u = p[u])
					C.push_back(x[u]);
				return pair<double, vector<int>>(ld, C);
			}
		}
	}

	void get_cycle(double& ld, int& s)
	{
		ld = INF;
		s = 0;
		vector<int> vis(n + 1, 0), dis(n + 1, 0);

		for (int i = 1; i <= n; i++)
		{
			vis[i] = !vis[i] ? 1 : -1;

			for (int u = i; vis[u] >= 0; u = p[u])
			{
				int v = p[u];
				if (!vis[v])
				{
					vis[v] = vis[u] + 1;
					dis[v] = dis[u] + c[u];
				}
				else if (vis[v] > 0)
				{
					double len = (double)(dis[u] + c[u] - dis[v]) / (vis[u] + 1 - vis[v]);
					if (ld > len)
					{
						ld = len;
						s = u;
					}
					break;
				}
			}

			for (int u = i; vis[u] >= 0; u = p[u])
				vis[u] = -1;
		}
	}

	void reverse_dfs(double ld, int s)
	{
		vector<vector<int>> pre(n + 1);
		for (int i = 1; i <= n; i++)
			if (i != s)
				pre[p[i]].push_back(i);

		queue<int> Q;
		for (Q.push(s); !Q.empty(); Q.pop())
		{
			int v = Q.front();
			for (int u : pre[v])
			{
				d[u] = d[v] + c[u] - ld;
				Q.push(u);
			}
		}
	}

	bool improve(double ld)
	{
		bool ans = false;
		for (const edge& E : e)
		{
			double nd = d[E.v] + E.c - ld;
			double dt = d[E.u] - nd;
			if (dt > 0)
			{
				ans |= dt > EPS;
				d[E.u] = nd;
				c[E.u] = E.c;
				p[E.u] = E.v;
				x[E.u] = E.x;
			}
		}
		return ans;
	}
};

struct mcc
{
	int n;
	vector<edge> e;

	mcc(int _n) : n(_n) {}

	void add_edge(int u, int v, int w, int c)
	{
		if (w <= 0)
			return;
		e.push_back(edge(u, v, w, c));
	}

	int operator()()
	{
		SCC S(n, e);
		vector<vector<edge>> f(S.cnt + 1);
		vector<int> idx(n + 1, 0), cnt(n + 1, 0);
		for (int i = 1; i <= n; i++)
			idx[i] = ++cnt[S.com[i]];
		for (const edge& E : e)
			if (S.com[E.u] == S.com[E.v])
				f[S.com[E.u]].push_back(edge(idx[E.u], idx[E.v], E.w, E.c));

		int ans = 0;
		for (int i = 1; i <= S.cnt; i++)
			if(!f[i].empty())// scc, 1 vertex, no edge, no cycle
				ans += mcc_scc_cycle(cnt[i], f[i]);// scc, >=1 vertex, >=1 edge, >=1 cycle
		return ans;
	}

	int mcc_scc_cycle(int n, vector<edge> &e)
	{
		for (int i = 0, m = e.size(); i < m; i++)
		{
			e.push_back(edge(e[i].v, e[i].u, 0, -e[i].c, i + m, i));
			e[i].x = i, e[i].r = i + m;
		}

		int ans = 0;
		while (1)
		{
			vector<edge> f;
			for (const edge &E : e)
				if (E.w > 0)
					f.push_back(E);
			pair<double, vector<int>> pa = howard(n, f)();

			if (pa.first > -EPS)
				break;

			const vector<int> &C = pa.second;
			int minw = INF;
			for (int x : C)
				minw = min(minw, e[x].w);
			for (int x : C)
			{
				ans += minw * e[x].c;
				e[x].w -= minw;
				e[e[x].r].w += minw;
			}
		}
		return ans;
	}
};

int main()
{
	int ncase;
	for (scanf("%d", &ncase); ncase > 0; ncase--)
	{
		int n, s, g, t, m;
		scanf("%d%d%d%d%d", &n, &s, &g, &t, &m);
		mcc M(++n);

		for (int i = 0, u; i < m; i++)
		{
			scanf("%d", &u);
			M.add_edge(u, n, n, 0);
		}
		scanf("%d", &m);
		for (int i = 0, u, v, w, c; i < m; i++)
		{
			scanf("%d%d%d%d", &u, &v, &w, &c);
			M.add_edge(u, v, w, c);
		}
		for (int i = 1; i <= n; i++)
			M.add_edge(i, i, n, 1);

		M.add_edge(n, s, n*n, -(t + 1));

		int ans = min(-M(), g);
		printf("%d\n", ans);
	}
}
