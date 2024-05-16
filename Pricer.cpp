#include "Pricer.h"
#include <cmath>

double Pricer::Price(const Market &mkt, Trade *trade) {
    double pv;
    std::cout<< "mkt: " << mkt << " trade!!!: " << trade->getType() << std::endl;
    if (trade->getType() == "AmericanOption" || trade->getType() == "EuropeanOption") {
        std::cout << "tree product" << " " << trade << std::endl;
        TreeProduct *treePtr = dynamic_cast<TreeProduct *>(trade);
        if (treePtr) { // check if cast is sucessful
            pv = PriceTree(mkt, *treePtr);
        }
    } else {
        double price; // get from market data
        price = mkt.getSpotPrice(trade->getType());
        std::cout << "not tree product, where spot price : " << price << " for type : " << trade->getType() << endl;
        pv = trade->Payoff(price);
    }
    return pv;
}

double BinomialTreePricer::PriceTree(const Market &mkt,
                                     const TreeProduct &trade) {
    // model setup
    double T = trade.GetExpiry() - mkt.asOf;
    double dt = T / nTimeSteps;
    /*
    get these data for the deal from market object
    */
    double stockPrice = 0;
    if (trade.getType() == "TreeProduct" || trade.getType() == "AmericanOption" || trade.getType() == "EuropeanOption") {
      std::cout << "underlying111: " << trade.getUnderlying() << std::endl;
      stockPrice = mkt.getSpotPrice(trade.getUnderlying());
    } else {
      stockPrice = mkt.getSpotPrice(trade.getType());
    }
    double vol = mkt.getVolatility(trade.getType());
    double rate = mkt.getRiskFreeRate();

    std::cout << "000get vol: " << vol << " get rate: " << rate << std::endl;

    ModelSetup(stockPrice, vol, rate, dt);

    for (int i = 0; i <= nTimeSteps; i++) {
        states[i] = trade.Payoff(GetSpot(nTimeSteps, i));
    }

    // price by backward induction
    for (int k = nTimeSteps - 1; k >= 0; k--)
        for (int i = 0; i <= k; i++) {
            // calculate continuation value
            double df = exp(-rate * dt);
            double continuation =
                df * (states[i] * GetProbUp() + states[i + 1] * GetProbDown());
            // calculate the option value at node(k, i)
            states[i] = trade.ValueAtNode(GetSpot(k, i), dt * k, continuation);
        }

    return states[0];
}

void CRRBinomialTreePricer::ModelSetup(double S0, double sigma, double rate,
                                       double dt) {
    double b = std::exp((2 * rate + sigma * sigma) * dt) + 1;
    u = (b + std::sqrt(b * b - 4 * std::exp(2 * rate * dt))) / 2 /
        std::exp(rate * dt);
    p = (std::exp(rate * dt) - 1 / u) / (u - 1 / u);
    currentSpot = S0;
}

void JRRNBinomialTreePricer::ModelSetup(double S0, double sigma, double rate,
                                        double dt) {
    u = std::exp((rate - sigma * sigma / 2) * dt + sigma * std::sqrt(dt));
    d = std::exp((rate - sigma * sigma / 2) * dt - sigma * std::sqrt(dt));
    p = (std::exp(rate * dt) - d) / (u - d);
    currentSpot = S0;
}
