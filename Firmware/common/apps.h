#ifndef APPLIST_H_
#define APPLIST_H_

enum modes {
	m_calc,
	m_mandelbrot,
	m_graph,
	m_solve_menu,
	m_info,
	m_error,
    m_applist
};

void applist_updateScreen();
void applist_buttonPress(int, int *);

#endif