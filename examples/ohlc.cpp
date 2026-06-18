/* Copyright (c) 2026 Steven Varga, Toronto, ON, Canada
 * MIT License — see LICENSE
 *
 * Candlestick (OHLC) example -> ohlc.svg.
 */
#include <plot/all>
#include <vector>
#include <algorithm>

int main(){
	std::vector<double> t, o, h, l, c;
	unsigned s = 4242u;
	auto rnd = [&]{ s = s*1664525u + 1013904223u; return double(s>>8 & 0xFFFF)/65536.0; };
	double price = 100.0;
	for(int i=0;i<40;++i){
		double open = price;
		double move = (rnd()-0.5)*6.0;
		double close = open + move;
		double high = std::max(open, close) + rnd()*2.0;
		double low  = std::min(open, close) - rnd()*2.0;
		t.push_back(i); o.push_back(open); h.push_back(high); l.push_back(low); c.push_back(close);
		price = close;
	}
	plot::ohlc(t,o,h,l,c,
		plot::title("OHLC candlestick"), plot::xlabel{"session"}, plot::ylabel{"price"},
		plot::width{720}, plot::height{420}).save("ohlc.svg");
	return 0;
}
